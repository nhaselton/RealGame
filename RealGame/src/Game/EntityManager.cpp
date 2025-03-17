#include "EntityManager.h"
#include "Physics\Physics.h"
#include "Renderer\DebugRenderer.h"

void CreateEntityManager() {
	entityManager.numEntities = 0;

	entityManager.numProjectiles = 0;
	entityManager.lastProjectileIndex = 0;
	entityManager.numEntities = 0;

	for ( u32 i = 0; i < MAX_ENTITIES; i++ )
		entityManager.entities[i].index = i;
}

//Takes an ActiveEntity from the pool allocator, returns a ptr to the entity and adds it to the activeHeadList
Entity* NewEntity() {
	if ( entityManager.numEntities == MAX_ENTITIES ) {
		LOG_ASSERT( LGS_GAME, "Too many entities, can not allocate more\n" );
		return 0;
	}
	//Find first free entity (TODO Better way?)
	for ( int i = 0; i < MAX_ENTITIES; i++ ) {
		StoredEntity* stored = &entityManager.entities[i];
		if ( stored->state != ACTIVE_INACTIVE )
			continue;

		entityManager.numEntities++;
		memset( &stored->entity, 0, MAX_ENTITY_SIZE );

		stored->entity.rotation = Quat( 1, 0, 0, 0 );
		stored->entity.animTimeScale = 1.0f;

		//Add Bounds
		stored->entity.bounds = &physics.entityColliders[i];
		memset( stored->entity.bounds, 0, sizeof( *stored->entity.bounds ) );
		stored->entity.bounds->canRaycast = true;

		physics.activeColliders[physics.numActiveColliders++] = &physics.entityColliders[i];
		stored->entity.bounds->owner = &stored->entity;

		stored->state = ACTIVE_ACTIVE;
		return &stored->entity;
	}

	return 0;
}

//Note: Make sure you manually remove boids.
void RemoveEntity( Entity* e ) {
	StoredEntity* storedEntity = ( StoredEntity* ) e;
	if( e->bounds )
		e->bounds->canRaycast = false;

	storedEntity->state = ACTIVE_WAIT_FOR_REMOVE;
	entityManager.removeEntities[entityManager.numRemoveEntities++] = storedEntity;

	//Todo Remove Bounds
	//Find bounds in active bounds list
	for ( int i = 0; i < physics.numActiveColliders; i++ ) {
		CharacterCollider* collider = physics.activeColliders[i];
		if ( collider == e->bounds ) {
			//Replace this one with one at end
			physics.activeColliders[i] = physics.activeColliders[--physics.numActiveColliders];
		}
	}
}


void UpdateProjectiles( ) {
	int currentMaxProjectile = 0;
	for ( int i = 0; i <= entityManager.lastProjectileIndex; i++ ) {
		Projectile* projectile = &entityManager.projectiles[i];
		if ( projectile->state != ACTIVE_ACTIVE )
			continue;

		currentMaxProjectile = i;
		projectile->collider.offset += projectile->velocity * dt;

		EntityCollisonQuery query;
		if ( PhysicsQueryIntersectEntities( &projectile->collider, &query ) ) {
			if ( projectile->OnCollision != 0 ) {
				projectile->OnCollision( projectile, query.entity );
			}
		}

		if( projectile->isLinear && projectile->staticImpactTime <= gameTime ) {
			if ( projectile->OnCollision )
				projectile->OnCollision( projectile, 0 );
		}
	}

	entityManager.lastProjectileIndex = currentMaxProjectile;
}

void ProjSweep(Vec3 pos, Vec3 vel, Vec3 bounds) {
	SweepInfo best{};

	//for( int i = 0; i < physics.numBrushes; i++ ) {
	//	if ( Sweep )
	//}

}

Projectile* NewProjectile( Vec3 pos, Vec3 velocity, Vec3 radius, bool linear ) {
	if ( entityManager.numProjectiles >= MAX_PROJECTILES ) {
		LOG_WARNING( LGS_GAME, "Can not spawn more projectiles.\n" );
		return 0;
	}

	for ( int i = 0; i < MAX_PROJECTILES; i++ ) {
		Projectile* projectile = &entityManager.projectiles[i];
		//Get Non active projectile
		if ( projectile->state == ACTIVE_ACTIVE )
			continue;

		memset( projectile, 0, sizeof( *projectile ) );

		projectile->state = ACTIVE_ACTIVE;
		entityManager.numProjectiles++;

		//Update last index if needed
		if ( i > entityManager.lastProjectileIndex )
			entityManager.lastProjectileIndex = i;

		projectile->collider.offset = pos;
		projectile->collider.bounds.width = radius;
		projectile->collider.bounds.center = Vec3( 0 );
		projectile->velocity = velocity; 
		projectile->spawnTime = gameTime;

		if( linear ) {
			SweepInfo info{};
			if( PhysicsQuerySweepStatic( pos, glm::normalize(velocity) * 200.0f, radius * 2.0f, &info) ) {
				projectile->isLinear = true;
				Vec3 fromTo = info.r3Point - pos;
				float dist = glm::length( fromTo  );
				Vec3 normal = fromTo / dist;

				float speed = glm::dot( velocity , normal );
				float time = info.eSpaceNearestDist / speed;
				projectile->staticImpactTime = gameTime + time;
			}
			else {
				//Dont let projectiles live forver
				projectile->staticImpactTime = gameTime + 10.0f;
				printf( "Fail" );
			}
		}

		return projectile;
	}

	LOG_ASSERT( LGS_GAME, "UNREACHABLE END OF NewProjectile()" );
	return 0;
}

void EntityManagerCleanUp() {
	for ( int i = 0; i < entityManager.numRemoveProjectiles; i++ )
		entityManager.projectiles[entityManager.removeProjectiles[i]].state = ACTIVE_INACTIVE;

	for( int i = 0; i < entityManager.numRemoveEntities; i++ ) {
		entityManager.removeEntities[i]->state = ACTIVE_INACTIVE;
		entityManager.numEntities--;
	}
	
	entityManager.numRemoveEntities = 0;
	entityManager.numRemoveProjectiles = 0;
}

void RemoveProjectile( Projectile* projectile ) {
	u64 projectileIndex = ( u64 ) projectile - ( u64 ) entityManager.projectiles;
	entityManager.removeProjectiles[entityManager.numRemoveProjectiles++] = (u16) projectileIndex;
	projectile->state = ACTIVE_WAIT_FOR_REMOVE;
}

void UpdateEntities() {
	for ( int i = 0; i < MAX_ENTITIES; i++ ) {
		StoredEntity* stored = &entityManager.entities[i];
		
		if ( stored->state != ACTIVE_ACTIVE )
			continue;

		if ( stored->entity.Update != 0 )
			stored->entity.Update( &stored->entity );
	}
}

void AnimateEntities() {
	for( int i = 0; i < MAX_ENTITIES; i++ ) {
		StoredEntity* stored = &entityManager.entities[i];

		if( stored->state != ACTIVE_ACTIVE )
			continue;

		if( stored->entity.renderModel != 0 )
			EntityAnimationUpdate( &stored->entity, dt );
	}
}
