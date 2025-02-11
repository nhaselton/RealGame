#pragma once
#include "def.h"
#include "Entity.h"

enum goblinStates_t {
	GOBLIN_CHASE = 0,
	GOBLIN_STAGGER = 1,
	//GOBLIN_IDLE (Once sees player never idles again)
	//GOBLIN_EXPLODE
};

enum goblinAnimations {
	GOBLIN_ANIM_RUN,
	GOBLIN_ANIM_STAGGER,
};

class Goblin : public Entity{
public:
	static Model* model;
	struct AudioSource* audioSource;
};

Goblin* CreateGoblin( Vec3 pos );
void GoblinUpdate( Entity* entity );
void GoblinOnHit( EntityHitInfo info );

void GoblinChase( Goblin* goblin );
void GoblinStagger( Goblin* goblin );