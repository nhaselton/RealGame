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
	if ( !entity->currentAnimation) {
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
	entity->pos = MoveAndSlide(entity->bounds, gravity, 0, true);

	if (GroundCheck(entity->pos, .15f, entity->bounds->bounds.width)) {
		entity->pos = SnapDown(entity->pos, entity->bounds->bounds.width, 1000.0f);
		entity->bounds->offset = entity->pos;
	}

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

void CreateDeadBody( RenderModel* model, SkeletonPose* pose, Vec3 pos, Quat rot, BoundsHalfWidth* bounds ) {
	RigidBody* deadBody = NewRigidBody();
	if( !deadBody )
		return;

	deadBody->pos = pos + bounds->center / 2.0f;
	deadBody->removeTime = gameTime + 10.0f;;
	deadBody->radius = bounds->width.x;
	deadBody->modelScale = model->scale.x;
	deadBody->rotOffset = rot;
	deadBody->model = model->model;
	deadBody->pose = Chaingunner::deadPose;
}

bool EntityApplyStagger( Entity* entity, int damage ) {
	entity->staggerNow += damage * entity->staggerPerDamage;
	if( entity->staggerNow >= entity->staggerAt ) {
		entity->staggerNow = 0;
		return true;
	}
	return false;
}

void DoorUpdate(Entity* entity ) {
	if( entityManager.keysDown == 3 ) {
		entity->pos -= Vec3( 0, 10 * dt, 0 );
	}
}

void DoorLoadKVP( void* door, char* key, char* value ) {
	TryEntityField( (Entity*) door, key, value );
	Door* ddoor = (Door*) door;
	ddoor->bounds->offset = ddoor->pos;
	ddoor->bounds->bounds.center = Vec3( 0 );
	ddoor->bounds->bounds.width = Vec3( 19.5,13.25,2. );
	ddoor->Update = DoorUpdate;
}
