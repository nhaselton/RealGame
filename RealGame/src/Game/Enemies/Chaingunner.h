#pragma once
#include "def.h"
#include "game/entity.h"
#include "Resources/Resources.h"

enum chaingunnerAnim_t {
	CG_ANIM_DEATH,
	CG_ANIM_IDLE,
	CG_ANIM_RUN,
	CG_ANIM_SHOOT,
	CG_ANIM_STAGGER
};

enum chaingunnerState_t {
	CG_IDLE = 0,
	CG_MOVING,
	CG_SHOOTING,
	CG_STAGGER,
	CG_DYING,
	CG_DEAD
};

class Chaingunner : public Entity {
public:
	float nextShootTime;
	float shootCooldown;
	float startShootingTime;
	float startMovingTime;
	float spread;

	class AudioSource* audioSource;

	static Model* model;
	static Model* projectileModel;

	static SkeletonPose* deadPose;
};


Chaingunner* CreateChaingunner(Vec3 pos);

void ChaingunnerUpdate(Entity* entity);
void ChaingunnerRecievedAnimEvent(Entity* entity, AnimationEvent* event);
void ChaingunnerShootBullet(Entity* entity);

void ChaingunnerIdle(Entity* entity);
void ChaingunnerShootingStart(Entity* entity);
void ChaingunnerShooting(Entity* entity);
void ChaingunnerDyingStart(Entity* entity);
void ChaingunnerDying(Entity* entity);
void ChaingunnerIdle(Entity* entity);
void ChaingunnerMoving(Entity* entity);
void ChaingunnerMovingStart(Entity* entity);
void ChaingunnerOnHit(EntityHitInfo info);
void ChaingunnerStagger(Entity* entity);
void ChaingunnerStaggerStart(Entity* entity);
void ChaingunnerLoadKVP(void* ent, char* key, char* value);
