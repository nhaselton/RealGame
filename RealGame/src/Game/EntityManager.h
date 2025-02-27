#pragma once
#include "def.h"
#include "Entity.h"
#include "Encounter.h"

//Rather than just storing an entity,
//Store the entity and then an extra buffer of MAX_ENTITIY_SIZE
//Do not edit layout of struct
struct StoredEntity {
	Entity entity;
	char entityExtra[MAX_ENTITY_SIZE - sizeof( Entity )];
	activeState_t state;
	u32 index; //Quick lookup of index.
};

enum spawnTarget_t {
	NONE,
	SPAWN_TARGET_POINT,
	SPAWN_TARGET_ZONE
};
struct SpawnTarget {
	spawnTarget_t type;
	encounterEnemies_t enemies; //Can be blank if only called from encounter
	char name[32];
	Vec3 pos;
	Vec3 size; //Incase of SPAWN_ZONE
};

class EntityManager {
public:
	int numEntities;

	//PoolArena entityArena;
	//ActiveEntity* activeHead;
	StoredEntity entities[MAX_ENTITIES];

	//Because projectiles have such a short lifespan, I'm just going to brute force this
	Projectile projectiles[MAX_PROJECTILES];
	int numProjectiles;
	int lastProjectileIndex; //How far into the array would it possibly have to go.

	//indices of projectiles that should be removed at the end of the frame
	u16 removeProjectiles[MAX_PROJECTILES];
	int numRemoveProjectiles;
	
	StoredEntity* removeEntities[MAX_ENTITIES];
	int numRemoveEntities;

	Trigger triggers[MAX_TRIGGERS];
	u32 numTriggers;

	//Quick access to player
	class Player* player;

	SpawnTarget spawnTargets[MAX_SPAWN_TARGETS];
	int numSpawnTargets;

	Encounter encounters[MAX_ENCOUNTERS];
	int numEncounters;
};
extern EntityManager entityManager;

void CreateEntityManager();

//Takes an ActiveEntity from the pool allocator, returns a ptr to the entity and adds it to the activeHeadList
Entity* NewEntity();
void RemoveEntity( Entity* entity );
void UpdateEntities();

Projectile* NewProjectile( Vec3 pos, Vec3 velocity, Vec3 radius, bool linear );
void RemoveProjectile( Projectile* projectile );
void UpdateProjectiles();

//Removes all inactive projectiles & (todo) entities
void EntityManagerCleanUp();
void AnimateEntities();

void TriggerTrigger( Trigger* trigger );