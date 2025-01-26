#pragma once
#include "def.h"

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

//For ray casts / entity entity collisions they use their AABB
//For Collision with static geometry it uses a Ellipse with the size of bounds.halfWidth
struct CharacterCollider {
	Vec3 offset;
	BoundsHalfWidth bounds;
};

class Entity {
public:
	Vec3 pos;
	Quat rotation;

	CharacterCollider bounds;
	RenderModel* renderModel;

	struct AnimationClip* currentAnimation;
	float currentAnimationTime; 
	float currentAnimationPercent; //t = [0,1]

	u32 state;
};

void EntityStartAnimation( Entity* entity, int index );
void EntityAnimationUpdate( Entity* entity, float dt );

void EntityMove( Entity* entity, Vec3 velocity );