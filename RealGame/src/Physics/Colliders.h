#pragma once
#include "def.h"

struct BoundsMinMax {
	Vec3 min;
	Vec3 max;
};

struct BoundsHalfWidth {
	Vec3 center;
	Vec3 width;
};

//For ray casts / entity entity collisions they use their AABB
//For Collision with static geometry it uses a Ellipse with the size of bounds.halfWidth
struct CharacterCollider {
	Vec3 offset;
	BoundsHalfWidth bounds;
	struct Entity* owner;
};