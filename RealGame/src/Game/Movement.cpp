#include "Movement.h"
#include "Renderer\DebugRenderer.h"
#include "Game\Entity.h"

inline Vec3 ProjectOnPlane( const Vec3& a, const Vec3& b ) {
	float dot = glm::dot( a, b );
	Vec3 proj = a - dot * b;
	return proj;
}

//Takes the current position of the bounds and snaps it onto the floor
Vec3 SnapDown( BoundsHalfWidth* bounds, float maxLength ) {
#if 0
	if ( maxLength == 0 ) maxLength = 100000.0f;

	HitInfo info;
	PhysicsQuerySweep( bounds, bounds->center, bounds->center - Vec3( 0, maxLength, 0 ), &info );

	//Close enough to ground
	if ( info.overlap < .1f )
		return bounds->center;

	if ( info.didHit ) {
		return info.point + info.normal * .001f;
	}

	//No hit
	return bounds->center;
#endif
	return Vec3( 0 );
}



Vec3 MoveAndSlide( BoundsHalfWidth* bounds, const Vec3& velocity, int numBounces ) {
#if 0
	float mag = glm::length( velocity );
	Vec3 dir = velocity / mag;

	//Copy of bounds that can be moved without touching the passed one
	BoundsHalfWidth currentBounds = *bounds;
	while ( mag > 0 ) {
		HitInfo info;
		PhysicsQuerySweep( &currentBounds, currentBounds.center, currentBounds.center + dir * mag, &info );

		//No Collision, safe to move
		if ( !info.didHit ) {
			currentBounds.center += dir * mag;
			DebugDrawBoundsHalfWidth( &currentBounds, Vec3( 0, 0, 1 ) );
			break;
		}

#if 1 //Stair stepping
		//if stair, has to be Horizontal
		if ( fabs( glm::dot( info.normal, Vec3( 0, 1, 0 ) ) < .1f ) ) {
			//Try moving the player up STEP_HEIGHT, moving it forward, then down and see if it lands on the ground and made it farther
			BoundsHalfWidth steppedPlayer{ currentBounds.center + Vec3( 0,STEP_HEIGHT,0 ), currentBounds.width };
			HitInfo stepInfo;

			PhysicsQuerySweep( &steppedPlayer, steppedPlayer.center, steppedPlayer.center + dir * mag, &stepInfo );

			if ( !stepInfo.didHit ) {
				currentBounds.center = steppedPlayer.center + dir * mag;
				currentBounds.center = SnapDown( &currentBounds, STEP_HEIGHT );
				break;
			}

			//This step went farther, make sure the player can still be snapped on the ground after
			if ( stepInfo.overlap < info.overlap ) {
				info = stepInfo;
				//Snapdown the final position
				info.point = SnapDown( &steppedPlayer, STEP_HEIGHT );
			}
		}
#endif

		//Did Collide, Move
		mag *= ( 1.0f - info.overlap );
		currentBounds.center = info.point + ( info.normal * .01f );
		dir = ProjectOnPlane( dir, info.normal );
		
		if ( numBounces == 0 )
			break;
		numBounces--;
	}
	return currentBounds.center;
#endif
	return Vec3( 0 );
}

void PlayerMovement( Entity* entity, Vec3 velocity ) {
#if 0
	BoundsHalfWidth bounds = *entity->bounds;
	bounds.center += entity->pos;

	HitInfo info;
	Vec3 gravity = Vec3( 0, -dt * 5.0f, 0 );
	PhysicsQuerySweep( &bounds, bounds.center, bounds.center + gravity, &info );

	//Gravity 
	if ( info.didHit ) {
		bounds.center = info.point + info.normal * .01f;
	}
	else
		bounds.center += gravity;

	//Move
	if ( velocity == Vec3( 0 ) ) {
		entity->pos = bounds.center;
		return;
	}
	velocity.y = 0;

	entity->pos = MoveAndSlide( &bounds, velocity, 3 );
#endif
}
