#include "EntityManager.h"
#include "Physics\Physics.h"
#include "Renderer\DebugRenderer.h"

void CreateEntityManager() {
	entityManager.numEntities = 0;
	CreatePoolArena( &entityManager.entityArena, sizeof( ActiveEntity ), 
		MAX_ENTITIES, malloc( MAX_ENTITIES * sizeof( ActiveEntity ) ),
		&globalArena, "Entity Manager"
	);
	entityManager.activeHead = 0;

	entityManager.numProjectiles = 0;
	entityManager.lastProjectileIndex = 0;

	for ( int i = 0; i < MAX_ENTITIES; i++ ) {
		ActiveEntity* activeEntityList = ( ActiveEntity* ) entityManager.entityArena.memory;
		ActiveEntity* activeEnt = &activeEntityList[i];
		activeEnt->index = i;
	}
}

//Takes an ActiveEntity from the pool allocator, returns a ptr to the entity and adds it to the activeHeadList
Entity* NewEntity() {
	if ( entityManager.numEntities == MAX_ENTITIES ) {
		LOG_ASSERT( LGS_GAME, "Too many entities, can not allocate more\n" );
		return 0;
	}

	//Get new entity
	ActiveEntity* ent = ( ActiveEntity* ) PoolArenaAllocate( &entityManager.entityArena );
	ent->next = entityManager.activeHead;
	entityManager.activeHead = ent;

	Entity* entity = ( Entity* ) ent->entity;
	memset( entity, 0, sizeof( Entity ) );
	entity->rotation = Quat( 1, 0, 0, 0 );
	
	//Get Bounds
	entity->bounds = &physics.colliders[ent->index];
	memset( entity->bounds, 0, sizeof( *entity->bounds ) );

	//Add Active Physics Collider
	physics.activeColliders[physics.numActiveColliders++] = &physics.colliders[ent->index];
	entity->bounds->owner = entity;
	
	return ( Entity* ) ent->entity;
}

void DestroyEntity( Entity* entity ) {
	ActiveEntity* prev = 0;
	for ( ActiveEntity* ent = entityManager.activeHead; ent != 0; ent = ent->next ) {
		
		if ( ( Entity* ) ent->entity == entity ) {
			//Head
			if ( !prev ) {
				entityManager.activeHead = ent->next;
			}
			else {
				prev->next = ent->next;
			}
			
			PoolArenaFree( &entityManager.entityArena, ent );
			return;
		}

		prev = ent;
		ent = ent->next;
	}

	LOG_ASSERT( LGS_GAME, "Could not find entity in list of entities\n" );
}

void UpdateProjectiles( ) {
	int currentMaxProjectile = 0;
	for ( int i = 0; i <= entityManager.lastProjectileIndex; i++ ) {
		Projectile* projectile = &entityManager.projectiles[i];
		if ( !projectile->active )
			continue;

		currentMaxProjectile = i;
		projectile->collider.offset += projectile->velocity * dt;

		DebugDrawAABB( projectile->collider.offset + projectile->collider.bounds.center, projectile->collider.bounds.width, 0.0f, BLUE );
		
		EntityCollisonQuery query;
		if ( PhysicsQueryIntersectEntities( &projectile->collider, &query ) ) {
			if ( projectile->OnCollision != 0 ) {
				projectile->OnCollision( projectile, query.entity );
			}
		}
	}

	entityManager.lastProjectileIndex = currentMaxProjectile;
}

Projectile* NewProjectile() {
	if ( entityManager.numProjectiles == MAX_PROJECTILES ) {
		LOG_WARNING( LGS_GAME, "Can not spawn more projectiles.\n" );
		return 0;
	}

	for ( int i = 0; i < MAX_PROJECTILES; i++ ) {
		Projectile* projectile = &entityManager.projectiles[i];
		//Get Non active projectile
		if ( projectile->active )
			continue;

		projectile->active = true;
		entityManager.numProjectiles++;

		//Update last index if needed
		if ( i > entityManager.lastProjectileIndex )
			entityManager.lastProjectileIndex = i;


		return projectile;
	}

	LOG_ASSERT( LGS_GAME, "UNREACHABLE END OF NewProjectile()" );
	return 0;
}