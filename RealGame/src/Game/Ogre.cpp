#include "Ogre.h"
#include "Entity.h"
#include "Renderer\Renderer.h"
#include "Resources\ModelManager.h"
#include "Physics/Physics.h"
#include "Renderer\DebugRenderer.h"
#include "EntityManager.h"
#include "Resources\ModelManager.h"

Model* rockModel;

Ogre* CreateOgre( Vec3 pos, Entity* player ) {
	//Ogre* entity = ( Ogre* ) ScratchArenaAllocateZero( &globalArena, KB( 1 ) );
	if ( !rockModel )
		rockModel = ModelManagerAllocate( &modelManager, "res/models/rock.glb" );

	Ogre* entity = ( Ogre* ) NewEntity();
	entity->player = player;
	entity->pos = pos;

	Model* model = ModelManagerAllocate( &modelManager, "res/models/ogre.glb" );
	EntityGenerateRenderModel( entity, model, &globalArena );
	entity->renderModel->scale = Vec3( 3 );

	entity->bounds->offset = pos;
	entity->bounds->bounds.center = Vec3( 0, 3, 0 );
	entity->bounds->bounds.width = Vec3( 3 );
	entity->bounds->owner = entity;
	
	entity->nextAttack = 0;
	entity->attackCooldown = 5.0f;

	entity->renderModel->model->animations[OGRE_ANIM_WALKING]->looping = true;

	Vec3 dirToPlayer = player->pos - entity->pos;
	dirToPlayer.y = 0;
	entity->rotation = glm::quatLookAt( -glm::normalize( dirToPlayer ), Vec3( 0, 1, 0 ) );

	entity->Update = OgreUpdate;
	entity->OnHit = OgreOnHit;

	entity->nextAttack = gameTime + 1.0f;
	entity->health = 12;
	entity->maxHealth = 12;
	entity->hasThrownRock = false;
	entity->hasSwiped = false;

	entity->state = ( ogreState_t ) OGRE_TAUNT;
	EntityStartAnimation( entity, OGRE_ANIM_ROARING );
	return entity;
}

void OgreTaunt( Entity* entity ) {
	//Wait for anim end
	if ( entity->currentAnimationPercent == 1.0f ) {
		Ogre* ogre = ( Ogre* ) entity;
		ogre->nextAttack = gameTime + 1.0f;
		OgreStartChase( entity );
	}
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
	
	entity->rotation = glm::quatLookAt( -dir, Vec3( 0, 1, 0 ) );
	entity->pos = MoveAndSlide( entity->bounds, dir * dist * dt, 0, true );
}

void OgreUpdate( Entity* entity ) {
	//Apply Gravity
	entity->pos = MoveAndSlide( entity->bounds, Vec3( 0, -10 * dt, 0 ), 0, true );

	switch ( ( ogreState_t ) entity->state ) {
		case OGRE_CHASE: OgreChase( entity ); break;
		case OGRE_DIE: OgreDie( entity ); break;
		case OGRE_SWIPE: OgreSwipe( entity ); break;
		case OGRE_TAUNT: OgreTaunt( entity ); break;
		case OGRE_THROW: OgreThrow( entity ); break;
	default:
		LOG_ASSERT( LGS_GAME, "Bad ogre state\n" );
	}

	EntityAnimationUpdate( entity, dt );
	//DebugDrawCharacterCollider( entity->bounds );
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
	Ogre* ogre = ( Ogre* ) entity;
	
	if ( entity->currentAnimationPercent > .37 && !ogre->hasThrownRock ) {
		ogre->hasThrownRock = true;
		
		Projectile* rock = NewProjectile();
		Vec3 rockSpawn = entity->pos + Vec3( 0, 4, 0 );;

		rock->collider.offset = rockSpawn;
		rock->collider.bounds.width = Vec3( 2.0f );
		rock->collider.bounds.center = Vec3( 0 );
		rock->collider.owner = entity;
		rock->velocity = glm::normalize( ( ogre->player->pos - rockSpawn ) ) * 10.0f;
		rock->owner = ogre;
		rock->model.model = rockModel;
		rock->model.scale = Vec3( 1 );
		rock->model.translation = Vec3( 0 );
		rock->OnCollision = OgreRockCallback;
	}

	if ( entity->currentAnimationPercent == 1.0f ) {
		ogre->nextAttack = gameTime + ogre->attackCooldown;
		ogre->hasThrownRock = false;
		OgreStartChase( entity );
	}
}

void OgreSwipe( Entity* entity ) {
	Ogre* ogre = ( Ogre* ) entity;
	if ( entity->currentAnimationPercent == 1.0f ) {
		ogre->nextAttack = gameTime + ogre->attackCooldown;
		ogre->hasSwiped = true;
		OgreStartChase( entity );
	}

	if ( ogre->currentAnimationPercent >= 0.5f && !ogre->hasSwiped ) {
		ogre->hasSwiped = true;

		Vec3 delta = ogre->player->pos - ogre->pos;
		delta.y = 0;

		float dist = glm::length( delta );
		if ( dist < 10 ) {
			EntityHitInfo info{};
			info.attacker = entity;
			info.victim = ogre->player;
			info.projectile = 0;
			info.damage = 1;

			ogre->player->OnHit( info );
		}
	}
}

void OgreOnHit( EntityHitInfo info ) {
	if ( info.victim->state == OGRE_DIE ) {
		return;
	}

	info.victim->health -= 1;

	if ( info.victim->health <= 0 )
		OgreStartDie( info.victim );
}

void OgreStartDie( Entity* entity ) {
	EntityStartAnimation( entity, OGRE_ANIM_DYING);
	entity->state = OGRE_DIE;
	entity->currentAnimationTime = .6f;
}

void OgreDie( Entity* entity ) {

}

void OgreRockCallback( class Projectile* projectile, class Entity* entity ) {
	if ( entity && entity->OnHit != 0 ) {
		EntityHitInfo info{};
		info.attacker = projectile->owner;
		info.damage = 10.0f;
		info.projectile = projectile;
		info.victim = entity;

		entity->OnHit( info );
	}
	RemoveProjectile( projectile );
}