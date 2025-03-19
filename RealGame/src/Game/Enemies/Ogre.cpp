#include "def.h"
#include "Game/Game.h"
#include "Renderer\Renderer.h"
#include "Resources\ModelManager.h"
#include "Physics/Physics.h"
#include "Renderer\DebugRenderer.h"
#include "Resources\ModelManager.h"

Model* Ogre::model;
Model* Ogre::projectileModel;
SkeletonPose* Ogre::deadPose;

Ogre* CreateOgre( Vec3 pos ) {
	//Ogre* entity = ( Ogre* ) ScratchArenaAllocateZero( &globalArena, KB( 1 ) );
	Ogre* entity = ( Ogre* ) NewEntity();
	CreateBoid(entity);
	entity->pos = pos;

	EntityGenerateRenderModel( entity, Ogre::model, &globalArena );
	entity->renderModel->scale = Vec3( 1.75 );

	entity->bounds->offset = pos;
	entity->bounds->bounds.center = Vec3( 0, 1.9, 0 );
	entity->bounds->bounds.width = Vec3( 1.5,1.62,1.5 );
	entity->bounds->owner = entity;
	entity->bounds->canRaycast = true;
	
	entity->nextAttack = 0;
	entity->attackCooldown = 5.0f;

	entity->renderModel->model->animations[OGRE_ANIM_WALKING]->looping = true;
	entity->Update = OgreUpdate;
	entity->OnHit = OgreOnHit;

	entity->nextAttack = gameTime + 1.0f;
	entity->health = 32;
	entity->maxHealth = 32;
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

void OgreUpdate( Entity* entity ) {
	//Apply Gravity
	entity->animTimeScale = 1.5f;
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
	Vec3 playerPos = entityManager.player->pos;
	playerPos.y = 0;
	
	//Check Attacks
	if ( ogre->nextAttack <= gameTime ) {
		float playerDistance = glm::length( playerPos - ogre->pos );
		//if ( playerDistance > 15.0f ) {
		//	OgreStartThrow(entity);
		//	return;
		//}
		//else 
		if ( playerDistance < 6 ) {
			OgreStartSwipe( entity );
			return;
		}
	}

	ogre->target = entityManager.player->pos;
	EntityLookAtPlayer(ogre);
	EntityMove(ogre, ogre->boidVelocity * 2.0f * dt);
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
		
		Vec3 rockSpawn = entity->pos + Vec3( 0, 4, 0 );;
		Vec3 velocity = glm::normalize( ( entityManager.player->pos - rockSpawn ) ) * 10.0f;
		Vec3 bounds = Vec3( 2.0f );

		Projectile* rock = NewProjectile( rockSpawn, velocity, bounds, true);

		if( rock ) {
			rock->collider.owner = entity;
			rock->owner = ogre;
			rock->model.model = Ogre::projectileModel;
			rock->model.scale = Vec3( 1 );
			rock->model.translation = Vec3( 0 );
			rock->OnCollision = OgreRockCallback;
		}
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

		Vec3 delta = entityManager.player->pos - ogre->pos;
		delta.y = 0;

		float dist = glm::length( delta );
		if ( dist < 10 ) {
			EntityHitInfo info{};
			info.attacker = entity;
			info.victim = entityManager.player;
			info.projectile = 0;
			info.damage = 10.0f;

			entityManager.player->OnHit( info );
		}
	}
}

void OgreOnHit( EntityHitInfo info ) {
	info.victim->health -= info.damage;

	if ( info.victim->health <= 0 )
		OgreStartDie( info.victim );
}

void OgreStartDie( Entity* entity ) {
	EntityStartAnimation( entity, OGRE_ANIM_DYING);
	entity->state = OGRE_DIE;
	entity->currentAnimationTime = .6f;
}

void OgreDie( Entity* entity ) {
	Ogre* ogre = (Ogre*) entity;
	if( entity->currentAnimationPercent == 1.0f ) {
		CreateDeadBody( ogre->renderModel, Ogre::deadPose, ogre->pos, ogre->rotation, &ogre->bounds->bounds );

		RemoveBoid( entity );
		RemoveEntity(entity);
	}
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

void OgreLoadKVP(void* _ogre, char* key, char* value) {
	Ogre* ogre = (Ogre*)_ogre;
	if (!TryEntityField(ogre, key, value)) {
		LOG_WARNING(LGS_GAME, "Ogrehas no kvp %s : %s", key, value);
	}
}
