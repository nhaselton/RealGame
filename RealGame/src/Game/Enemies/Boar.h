#pragma once
#include "def.h"
#include "game/entity.h"
#include "Resources/Resources.h"

enum boarAnim_t {
	BOAR_ANIM_CHARGE,
	BOAR_ANIM_HEADBUTT,
	BOAR_ANIM_IDLE,
	//BOAR_ANIM_DEATH,
	//BOAR_ANIM_STAGGER
};

enum boarState_t{
	BOAR_IDLE,
	BOAR_CHARGE,
	BOAR_STAGGER,
	BOAR_HEADBUTT
};

class Boar : public Entity {
public:
	static Model* model;
	static SkeletonPose* deadPose;
	Vec3 startVel;
};

Boar* CreateBoar(Vec3 pos);

void BoarStartCharge(Entity* entity);
void BoarCharge(Entity* entity);
void BoarStagger(Entity* entity);

void BoarUpdate(Entity* entity);
void BoarHit(EntityHitInfo info);
void BoarRecievedAnimationEvent(Entity* entity, AnimationEvent* event);
void BoarLoadKVP(void* ent, char* key, char* value);