#pragma once
#include "def.h"
#include "physics/Colliders.h"

struct BoundsHalfWidth;
struct SkeletonPose;

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
	bool active; //Could replace this with a long bitflag list in entitymanager
	//Probably should rename from character collider
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

	CharacterCollider* bounds;
	RenderModel* renderModel;

	struct AnimationClip* currentAnimation;
	float currentAnimationTime; 
	float currentAnimationPercent; //t = [0,1]

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