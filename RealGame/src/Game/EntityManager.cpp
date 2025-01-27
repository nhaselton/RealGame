#include "EntityManager.h"
#include "Physics\Physics.h"

void CreateEntityManager() {
	entityManager.numEntities = 0;
	CreatePoolArena( &entityManager.entityArena, sizeof( ActiveEntity ), 
		MAX_ENTITIES, malloc( MAX_ENTITIES * sizeof( ActiveEntity ) ),
		&globalArena, "Entity Manager"
	);
	entityManager.activeHead = 0;

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
