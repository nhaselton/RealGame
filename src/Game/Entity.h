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

class Entity {
public:
	Vec3 pos;
	Quat rotation;

	//Bounds center should be an offset of the Entity position.
	//Call Entity Query To generate correct bounds?
	BoundsHalfWidth* bounds;
	RenderModel* renderModel;

	struct AnimationClip* currentAnimation;
	float currentAnimationTime; 
	float currentAnimationPercent; //t = [0,1]

	u32 state;
};

void EntityStartAnimation( Entity* entity, int index );
void EntityAnimationUpdate( Entity* entity, float dt );