#pragma once
#include "def.h"
#include "game/entity.h"

class Wizard : public Entity {
public:
	static Model* model;
};

Wizard* CreateWizard( Vec3 pos );