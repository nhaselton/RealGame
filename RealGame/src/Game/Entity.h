#pragma once
#include "def.h"
#include "physics/Colliders.h"

struct BoundsHalfWidth;
struct SkeletonPose;

enum activeState_t {
	ACTIVE_INACTIVE = 0,
	ACTIVE_WAIT_FOR_REMOVE = 1, //removed at end of frame
	ACTIVE_ACTIVE = 2,
};

struct RenderModel {
	class Model* model;
	SkeletonPose* pose;
	//todo pose

	Vec3 translation;
	Quat rotation;
	Vec3 scale;
};
//Projectiles are completely separated from entities becuase they share no functionality besides the charactercollider
//In ray casts, shape casts, etc. the projectile is never queried against 
// (TODO how to handle few exceptions)
class Projectile {
public:
	activeState_t state;
	//I do want it to sweep
	Vec3 velocity;
	class Entity* owner;

	CharacterCollider collider;
	void ( *OnCollision ) ( Projectile* projectile, Entity* entity );

	RenderModel model;

	//todo float impactTime
	//hitCallback?
	//Damage?
	//RenderModel
};

class Entity {
public:
	Vec3 pos;
	Quat rotation;

	Vec3 boidVelocity;

	CharacterCollider* bounds;
	RenderModel* renderModel;

	struct AnimationClip* currentAnimation;
	float currentAnimationTime; 
	float currentAnimationPercent; //t = [0,1]
	float animTimeScale;//how fast should the anims be

	u32 state;

	int health;
	int maxHealth;

	void ( *Update ) ( class Entity* entity );
	void ( *OnHit ) ( struct EntityHitInfo info );
};
//Init
void EntityGenerateRenderModel( Entity* entity, class Model* model, ScratchArena* arena );
//Animation
void EntityStartAnimation( Entity* entity, int index );
void EntityAnimationUpdate( Entity* entity, float dt );
//Runtime
void EntityMove( Entity* entity, Vec3 velocity );
void EntityLookAtPlayer( Entity* entity );