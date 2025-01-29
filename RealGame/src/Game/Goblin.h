#pragma once
#include "def.h"
#include "Entity.h"

enum goblinStates_t {
	GOBLIN_CHASE,
	//GOBLIN_STAGGER,
	//GOBLIN_IDLE (Once sees player never idles again)
	//GOBLIN_EXPLODE
};

class Goblin : public Entity{
public:
	static Model* model;
};

Goblin* CreateGoblin( Vec3 pos );