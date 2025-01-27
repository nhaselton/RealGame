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

	void ( *Update ) ( class Entity* entity );
	void ( *OnHit ) ( class Entity* entity, float damage );
};

void EntityStartAnimation( Entity* entity, int index );
void EntityAnimationUpdate( Entity* entity, float dt );

void EntityMove( Entity* entity, Vec3 velocity );