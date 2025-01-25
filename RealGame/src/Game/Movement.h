#pragma once
#include "def.h"
struct BoundsMinMax;
struct BoundsHalfWidth;
struct Entity;

//Returns the final position of where the center of the bounds should be
Vec3 MoveAndSlide( BoundsHalfWidth* bounds, const Vec3& velocity, int numBounces );
//Returns final player position
void PlayerMovement( Entity* entity, Vec3 velocity );
