#include "game.h"
#include "Entity.h"
#include "Physics\Physics.h"
#include "Resources/ModelManager.h"
void EntityStartAnimation( Entity* entity, int index ) {
	entity->currentAnimation =  entity->renderModel->model->animations[index];
	entity->currentAnimationTime = 0;
	entity->currentAnimationPercent = 0;
}

void PollAnimationEvents( Entity* entity ) {
	for( int i = 0; i < entity->currentAnimation->numEvents; i++ ) {
		AnimationEvent* event = &entity->currentAnimation->events[i];
		if( entity->currentAnimationTime > event->time && entity->lastAnimationTime < event->time) {
			if( entity->RecievedAnimationEvent ) {
				entity->RecievedAnimationEvent( entity, event );
			}
		}
	}
}

void EntityAnimationUpdate( Entity* entity, float dt ) {
	if ( !entity->currentAnimation ) {
		AnimatePose( 0, 0, entity->renderModel->pose );
		UpdatePose( entity->renderModel->pose->skeleton->root, Mat4( 1.0 ), entity->renderModel->pose );
		return;
	}

	entity->currentAnimationTime += dt * entity->animTimeScale;

	if ( entity->currentAnimationTime > entity->currentAnimation->duration ) {
		if ( entity->currentAnimation->looping )
			entity->currentAnimationTime -= entity->currentAnimation->duration;
		else
			entity->currentAnimationTime = entity->currentAnimation->duration	;
	}

	entity->currentAnimationPercent = entity->currentAnimationTime / entity->currentAnimation->duration;

	AnimatePose( entity->currentAnimationTime, entity->currentAnimation, entity->renderModel->pose );
	UpdatePose( entity->renderModel->model->skeleton->root, Mat4( 1.0 ), entity->renderModel->pose );
	PollAnimationEvents( entity );

	entity->lastAnimationTime = entity->currentAnimationTime;
}


void EntityMove( Entity* entity, Vec3 velocity ) {
	Vec3 gravity( 0,  -10.0f * dt , 0 );
	velocity.y = 0;

	entity->pos = MoveAndSlide( entity->bounds, velocity, 3, true );
	entity->pos = MoveAndSlide( entity->bounds, gravity, 0, true );

}

//Theres gotta be a better way probably idk
void EntityGenerateRenderModel( Entity* entity, Model* model, ScratchArena* arena ) {
	assert( model );
	entity->renderModel = (RenderModel*) ScratchArenaAllocate( arena, sizeof( RenderModel ) );
	entity->renderModel->rotation = Quat( 1, 0, 0, 0 );
	entity->renderModel->scale = Vec3( 1 );
	entity->renderModel->translation = Vec3( 0 );

	entity->renderModel->model = model;
	entity->renderModel->pose = (SkeletonPose*) ScratchArenaAllocate( arena, sizeof( SkeletonPose ) );
	entity->renderModel->pose->globalPose = 
		( Mat4* ) ScratchArenaAllocate( arena,  model->skeleton->numBones *  sizeof( Mat4) );
	entity->renderModel->pose->pose = 
		( JointPose* ) ScratchArenaAllocate( arena, model->skeleton->numNodes * sizeof( JointPose ) );
	entity->renderModel->pose->skeleton = model->skeleton;
}

void EntityLookAtPlayer( Entity* entity ) {
	Vec3 dir = entityManager.player->pos - entity->pos;
	dir.y = 0;//Do not move on vertical plane
	float dist = glm::length( dir );
	dir /= dist;
	
	float speed = 1.5f;
	dist = glm::min( dist, speed );
	
	entity->rotation = glm::quatLookAt( -dir, Vec3( 0, 1, 0 ) );
}


Model* DefLoadModel( const char* path ) {
	u32 len = 0;
	char* buffer = 0;
	if( !TempDumpFile( path, &buffer, &len ) ) {
		LOG_ERROR( LGS_RENDERER, "Could not load modeldef %s\n", path );
		return 0;
	}
	Parser parser( buffer, len );
	parser.ReadToken();

	parser.ExpectedTokenTypePunctuation( '{' );
	parser.ExpectedTokenString( "model" );
	char glbPath[MAX_PATH_LENGTH]{};
	parser.ParseString( glbPath, MAX_PATH_LENGTH );


	//Check if we already got the model (Can happen during reloads)
	Model* model = ModelManagerGetModel( glbPath, false );
	//If not then load it
	if( !model )
		model = ModelManagerAllocate( &modelManager, glbPath );

	//Make sure it actually loaded that time
	if( !model ) {
		LOG_ERROR( LGS_GAME, "No Wizard Model at %s\n", glbPath );
		return 0;
	}
	ModelInfo* modelInfo = ( ModelInfo* ) ( ( char* ) model - 8 );
	//Editing animations

	char key[MAX_NAME_LENGTH]{};
	char value[MAX_NAME_LENGTH]{};

	if( parser.GetCurrent().subType == '{' ) {
		parser.ReadToken();

		while( 1 ) {
			if( parser.GetCurrent().subType == '}' ) {
				break;
			}
			if( !LoadKeyValue( &parser, key, value ) )
				return 0;;

			if( !strcmp( key, "animation" ) ) {
				//Find Animation
				AnimationClip* animation = ModelFindAnimation( model, value );
				if( !animation ) {
					LOG_ERROR( LGS_GAME, "Could not find animation %s for model %s in %s\n", value, model->path, path );
					return 0;
				}
				parser.ExpectedTokenTypePunctuation( '{' );

				if( !LoadKeyValue( &parser, key, value ) )
					return 0;
				if( !strcmp( "numevents", key ) ) {
					int numEvents = atoi( value );
					//If hot reloading, we dont need to reallocate memory.
					//Just override it
					if( animation->numEvents != numEvents ) {
						animation->numEvents = numEvents;
						animation->events = ( AnimationEvent* ) ScratchArenaAllocate( &modelInfo->arena, animation->numEvents * sizeof( AnimationEvent ) );
					}
					for( int i = 0; i < animation->numEvents; i++ ) {
						AnimationEvent* event = &animation->events[i];
						parser.ExpectedTokenTypePunctuation( '{' );
						while( 1 ) {
							if( parser.GetCurrent().subType == '}' ) {
								parser.ReadToken();
								break;
							}

							if( !LoadKeyValue( &parser, key, value ) )
								return 0;
							if( !strcmp( "type", key ) ) {
								if( !strcmp( "SHOOT_PROJECTILE", value ) ) {
									event->type = ANIM_EVENT_SHOOT_PROJECTILE;
								}
								else if( !strcmp( "MELEE_ATTACK", value ) ) {
									event->type = ANIM_EVENT_MELEE_ATTACK;
								}
								else {
									LOG_WARNING( LGS_GAME, "Unkown Animation Event type %s\n", value );
									//return 0;
								}
							}
							else if( !strcmp( "time", key ) ) {
								event->time = atof( value );
							}
							else {
								LOG_WARNING( LGS_GAME, "Unkown anim event arg %s\n" );
								return 0;
							}
						}
					}
					//End of Animation
					parser.ExpectedTokenTypePunctuation( '}' );
				}
			}
		}
	}
	return model;
}