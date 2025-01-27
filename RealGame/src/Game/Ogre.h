#pragma once
#include "def.h"
#include "Entity.h"

enum ogreState_t {
	OGRE_TAUNT,
	OGRE_CHASE,
	OGRE_THROW,
	OGRE_SWIPE,
	OGRE_DIE
};

//Do not touch these. It is the model's information
enum ogreAnimation_t {
	OGRE_ANIM_DYING,
	OGRE_ANIM_JUMPING,
	OGRE_ANIM_ROARING,
	OGRE_ANIM_SWIPING,
	OGRE_ANIM_THROWING,
	OGRE_ANIM_WALKING
};

class Ogre : public Entity {
public:
	float nextAttack;
	float attackCooldown;
	Entity* player;
};

Entity* CreateOgre( Vec3 pos, Entity* player );

void OgreUpdate( Entity* entity );
void OgreOnHit( Entity* entity, float damage );

//Ogre States
void OgreMove( Entity* entity, Vec3 target );
void OgreTaunt( Entity* entity );
void OgreStartChase( Entity* entity );
void OgreChase(Entity * entity);
void OgreStartThrow( Entity* entity );
void OgreStartSwipe( Entity* entity );
void OgreSwipe( Entity* entity );
void OgreThrow( Entity* entity );
void OgreStartDie(Entity* entity);
void OgreDie( Entity* entity );
