#pragma once
#include "def.h"
#include "Entity.h"

struct ActiveEntity {
	ActiveEntity* next;
	//This allows entities to be in a simple pool array of the same size
	char entity[2048];
	u16 pad;
	//NOTE: Do not let index be in the first 8 bytes of this struct. The next* is stored there for the pool allocator and will break
	int index;
};

class EntityManager {
public:
	int numEntities;

	PoolArena entityArena;
	ActiveEntity* activeHead;

	//Because projectiles have such a short lifespan, I'm just going to brute force this
	Projectile projectiles[MAX_PROJECTILES];
	int numProjectiles;
	int lastProjectileIndex; //How far into the array would it possibly have to go.

	//Quick access to player
	class Player* player;
};
extern EntityManager entityManager;

void CreateEntityManager();

//Takes an ActiveEntity from the pool allocator, returns a ptr to the entity and adds it to the activeHeadList
Entity* NewEntity();
void DestroyEntity( Entity* entity );

Projectile* NewProjectile();
void UpdateProjectiles();

