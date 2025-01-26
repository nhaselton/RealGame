#include "Ogre.h"
#include "Entity.h"
#include "Renderer\Renderer.h"
#include "Resources\ModelManager.h"
#include "Physics/Physics.h"
#include "Renderer\DebugRenderer.h"

Entity* CreateOgre( Vec3 pos, Entity* player ) {
	Ogre* entity = ( Ogre* ) ScratchArenaAllocateZero( &globalArena, KB( 1 ) );
	entity->player = player;
	entity->pos = pos;

	entity->renderModel = (RenderModel*) ScratchArenaAllocateZero( &globalArena, sizeof( RenderModel ) );
	entity->renderModel->model = ModelManagerAllocate( &modelManager, "res/models/ogre.glb" );
	entity->renderModel->scale = Vec3(3 );
	entity->renderModel->rotation = Quat( 1, 0, 0, 0 );
	entity->renderModel->translation = Vec3( 0 );

	entity->renderModel->pose = (SkeletonPose*) ScratchArenaAllocateZero( &globalArena, sizeof( SkeletonPose ) );
	entity->renderModel->pose->globalPose = ( Mat4* ) ScratchArenaAllocateZero( &globalArena, entity->renderModel->model->skeleton->numNodes * sizeof( Mat4 ) );
	entity->renderModel->pose->pose = ( JointPose* ) ScratchArenaAllocateZero( &globalArena, entity->renderModel->model->skeleton->numNodes * sizeof( JointPose ) );
	entity->renderModel->pose->skeleton = entity->renderModel->model->skeleton;

	entity->bounds.offset = pos;
	entity->bounds.bounds.center = Vec3( 0, 3, 0 );
	entity->bounds.bounds.width = Vec3( 3 );
	

	//entity->bounds = PhysicsCreateDynamicBody( entity, Vec3( 0, 3.5, 0 ), Vec3( 3.5 ) );
	//entity->bounds = ( BoundsHalfWidth* ) ScratchArenaAllocate( &globalArena, sizeof( BoundsHalfWidth ) );
	//entity->bounds->center = Vec3( 0, 3.5, 0 );
	//entity->bounds->width = Vec3( 7, 7, 7 );

	entity->rotation = Quat( 1, 0, 0, 0 );

	entity->nextAttack = 0;
	entity->attackCooldown = 5.0f;

	entity->renderModel->model->animations[OGRE_ANIM_WALKING]->looping = true;

	//entity->state = ( ogreState_t ) OGRE_TAUNT;
	//EntityStartAnimation( entity, OGRE_ANIM_ROARING );
	OgreStartChase( entity );
	return entity;
}

void OgreMove( Entity* entity, Vec3 target ) {
	if ( target == entity->pos )
		return;

	Vec3 dir = target - entity->pos;
	dir.y = 0;//Do not move on vertical plane
	float dist = glm::length( dir );
	dir /= dist;

	float speed = 1.5f;
	dist = glm::min( dist, speed );
	
	//entity->pos += dir * dist * dt;
	entity->pos = MoveAndSlide( &entity->bounds, dir * dist * dt, 1, true );
	entity->rotation = glm::quatLookAt( -dir, Vec3( 0, 1, 0 ) );
}

void OgreTaunt( Entity* entity ) {
	//Wait for anim end
	if ( entity->currentAnimationPercent == 1.0f )
		OgreStartChase(entity);
}

void OgreUpdate( Entity* entity ) {
	//Apply Gravity
	entity->pos = MoveAndSlide( &entity->bounds, Vec3( 0, -10 * dt, 0 ), 1, true );

	switch ( ( ogreState_t ) entity->state ) {
		case OGRE_CHASE: OgreChase( entity ); break;
		case OGRE_DIE: break;
		case OGRE_SWIPE: OgreSwipe( entity ); break;
		case OGRE_TAUNT: OgreTaunt( entity ); break;
		case OGRE_THROW: OgreThrow( entity ); break;
	default:
		LOG_ASSERT( LGS_GAME, "Bad ogre state\n" );
	}

	EntityAnimationUpdate( entity, dt );
	DebugDrawCharacterCollider( &entity->bounds );
}

void OgreStartChase( Entity* entity ) {
	EntityStartAnimation( entity, OGRE_ANIM_WALKING );
	entity->state = OGRE_CHASE;
}

void OgreChase( Entity* entity ) {
	Ogre* ogre = ( Ogre* ) entity;
	Vec3 playerPos = ogre->player->pos;
	playerPos.y = 0;
	
	//Check Attacks
	if ( ogre->nextAttack <= gameTime ) {
		float playerDistance = glm::length( playerPos - ogre->pos );
		if ( playerDistance > 15.0f ) {
			OgreStartThrow(entity);
			return;
		}
		else if ( playerDistance < 8 ) {
			OgreStartSwipe( entity );
			return;
		}
	}

	OgreMove(entity, playerPos );
}

void OgreStartThrow( Entity* entity ) {
	Ogre* ogre = ( Ogre * ) entity;
	entity->state = OGRE_THROW;
	EntityStartAnimation( entity, OGRE_ANIM_THROWING );
}

void OgreStartSwipe( Entity* entity ) {
	entity->state = OGRE_SWIPE;
	EntityStartAnimation( entity, OGRE_ANIM_SWIPING );
}

void OgreThrow( Entity* entity ) {
	if ( entity->currentAnimationPercent == 1.0f ) {
		Ogre* ogre = ( Ogre* ) entity;
		ogre->nextAttack = gameTime + ogre->attackCooldown;
		OgreStartChase( entity );
	}
}

void OgreSwipe( Entity* entity ) {
	if ( entity->currentAnimationPercent == 1.0f ) {
		Ogre* ogre = ( Ogre* ) entity;
		ogre->nextAttack = gameTime + ogre->attackCooldown;
		OgreStartChase( entity );
	}
}
