#include "Physics.h"
#include "Resources\Level.h"
#include "Renderer/DebugRenderer.h"
#include "game/Entity.h"
#include "Renderer\Renderer.h"

void PhysicsInit() {
	memset( physics.activeColliders, MAX_ENTITIES, MAX_ENTITIES * sizeof( physics.activeColliders[0] ) );
}

void PhysicsLoadLevel( Level* level, NFile* file ) {
	physics.numBrushes = NFileReadU32( file );
	u32 numFaces = NFileReadU32( file );
	u32 numBrushes = NFileReadU32( file );
	u32 numTriangles = NFileReadU32( file );
	u32 numVertices = NFileReadU32( file );

	physics.brushes = ( Brush* ) ScratchArenaAllocate( &level->arena, numBrushes * sizeof( Brush ) );
	Polygon* faces = ( Polygon* ) ScratchArenaAllocate( &level->arena, numFaces * sizeof( Polygon ) );
	BrushTri* triangles = ( BrushTri* ) ScratchArenaAllocate( &level->arena, numTriangles * sizeof( BrushTri ) );
	Vec3* vertices = ( Vec3* ) ScratchArenaAllocate( &level->arena, numVertices * sizeof( Vec3 ) );

	NFileRead( file, physics.brushes, sizeof( Brush ) * numBrushes );
	NFileRead( file, faces, sizeof( Polygon ) * numFaces );
	NFileRead( file, triangles, sizeof( BrushTri ) * numTriangles );
	NFileRead( file, vertices, sizeof( Vec3 ) * numVertices );

	for ( int i = 0; i < physics.numBrushes; i++ ) {
		physics.brushes[i].polygons = faces + ( int ) physics.brushes[i].polygons;
		physics.brushes[i].vertices = vertices + ( int ) physics.brushes[i].vertices;
	}

	for ( int i = 0; i < numFaces; i++ ) {
		faces[i].triangles = triangles + ( int ) faces[i].triangles;
	}

	NFileRead( file, &physics.staticBVH, sizeof( BVHTree ) );
	physics.staticBVH.nodes = ( BVHNode* ) ScratchArenaAllocate( &level->arena, physics.staticBVH.numNodes * sizeof( BVHNode ) );
	NFileRead( file, physics.staticBVH.nodes, sizeof( BVHNode ) * physics.staticBVH.numNodes );
}

inline Vec3 ProjectOnPlane( const Vec3& planeNormal, const Vec3& vector ) {
	// Normalize the plane normal
	float normalLength = glm::length( planeNormal );
	Vec3 normalizedNormal = planeNormal * ( 1.0f / normalLength );

	// Calculate the projection of the vector onto the plane normal
	float projectionLength = glm::dot( vector, normalizedNormal );
	Vec3 projection = normalizedNormal * projectionLength;
	
// Subtract the projection from the original vector to get the projection on the plane
	return vector - projection;

}

//https://www.peroxide.dk/papers/collision/collision.pdf
//https ://arxiv.org/pdf/1211.0059
//https://www.youtube.com/watch?v=YR6Q7dUz2uk&t=425s
#define VERY_SMALL_DISTANCE .0000005f //Increasing this will break high fps
Vec3 MoveAndSlide( CharacterCollider* cc, Vec3 velocity, int maxBounces, bool adjustCharacterController ) {
	Vec3 startPos = cc->bounds.center + cc->offset;
	Vec3 startVel = velocity;

	Vec3 pos = startPos;

	if ( glm::length2( velocity ) < VERY_SMALL_DISTANCE ) return pos;

	//Slight Epslion to keep from penetrating walls due to fp error
	float skinWidth = .015f;
	Vec3 r = cc->bounds.width - skinWidth;
	//float r = 1.0f + ( -skinWidth );

	int bounces = 0;
	do {
		SweepInfo info{};

		//if ( !BruteCastSphere( pos, velocity, cc->bounds.width, &info ) ) {
		if ( !PhysicsQuerySweepStatic( pos, velocity, cc->bounds.width, &info ) ) {
			pos += velocity;
			break;
		}

		Vec3 point = info.r3Position + info.r3Velocity * info.t;
		//if ( glm::length( velocity ) < .01f )
		//	break;


		Vec3 slidePlaneOrigin = WorldFromEllipse( info.eSpaceIntersection, info.radius );
		Vec3 slidePlaneNormal = glm::normalize( point - slidePlaneOrigin );
		float slidePlaneDist = glm::dot( slidePlaneNormal, slidePlaneOrigin );

		Vec3 velToSurface = glm::normalize( velocity ) * ( glm::length( point - pos ) - skinWidth );
		Vec3 remaining = velocity - velToSurface;

		point = pos + velToSurface;
		assert( !glm::any( glm::isnan( point ) ) );
		pos = point;

		float mag = glm::length( remaining );
		//Note: Can probably use info.r3Norm as the plane
		remaining = ProjectOnPlane( slidePlaneNormal, remaining );


		if ( glm::length2( remaining ) == 0.0f )
			break;
		remaining = glm::normalize( remaining ) * mag;


		//assert( !glm::any( glm::isnan( remaining ) ) );
		pos = point;
		velocity = remaining;
	} while ( bounces++ < maxBounces );

	//We only want to update the offset, not the local position of the bounds
	pos -= cc->bounds.center;
	Vec3 finalPos = pos;

	//DebugDrawAABB( pos, Vec3( 1, 2, 1 ), 0, GREEN );
	//DebugDrawCharacterCollider( characterController, GREEN );

	if ( adjustCharacterController )
		cc->offset = finalPos;

	return finalPos;
}

bool PointInTriangle( const Vec3& q, Vec3 a, Vec3 b, Vec3 c ) {
	//Translate Triangle so point is in center
	a -= q;
	b -= q;
	c -= q;

	Vec3 u = glm::cross( b, c );
	Vec3 v = glm::cross( c, a );
	if ( glm::dot( u, v ) < 0.0f ) return 0;
	Vec3 w = glm::cross( a, b );
	if ( glm::dot( u, w ) < 0.0f ) return 0;
	return 1;
}

bool SolveQuadratic( float a, float b, float c, float* lowest ) {
	float det = b * b - 4.0f * a * c;
	if ( det < 0.0f )
		return false;

	float sqrtD = sqrtf( det );
	float r1 = ( -b - sqrtD ) / ( 2 * a );
	float r2 = ( -b + sqrtD ) / ( 2 * a );

	if ( r1 > r2 ) {
		float t = r1;
		r1 = r2;
		r2 = t;
	}

	if ( r1 > 0 ) {
		*lowest = r1;
		return true;
	}

	if ( r2 > 0 ) {
		*lowest = r2;
		return true;
	}

	return false;
}
Plane PlaneFromTri(const Vec3& a, const Vec3& b, const Vec3& c) {
	Vec3 normal = glm::cross( b - a, c - a );
	normal = glm::normalize( normal );
	Plane p;
	p.n = normal;
	p.d = glm::dot( normal, a );;
	return p;
}

Vec3 EllipseFromWorld(const Vec3& point, const Vec3& radius) {
	Vec3 out;
	out.x = point.x / radius.x;
	out.y = point.y / radius.y;
	out.z = point.z / radius.z;
	return out;
}

Vec3 WorldFromEllipse( const Vec3& point, const Vec3& radius ) {
	Vec3 out;
	out.x = point.x * radius.x;
	out.y = point.y * radius.y;
	out.z = point.z * radius.z;
	return out;
}

//Note: Gives Output in terms of EllipseSpace
bool TestTriangleEllipse( SweepInfo* sInfo, Vec3 a, Vec3 b, Vec3 c ) {
	//EllipsoidSpace
	a = EllipseFromWorld( a, sInfo->radius );
	b = EllipseFromWorld( b, sInfo->radius );
	c = EllipseFromWorld( c, sInfo->radius );

	Plane triPlane = PlaneFromTri( a, b, c );
	if ( glm::dot( sInfo->velocity, triPlane.n ) > 0 )
		return false;


	float signedDistance = glm::dot( triPlane.n, sInfo->basePoint ) - triPlane.d;//glm::dot( polygon->n, pos ) - polygon->d;
	float NdotV = glm::dot( sInfo->velocity, triPlane.n );//glm::dot( velocity, polygon->n );

	float t0 = ( 1 - signedDistance ) / NdotV;
	float t1 = ( -1 - signedDistance ) / NdotV;

	float t = 1.0f;

	bool embed = false;
	if ( NdotV == 0.0f ) {
		if ( fabs( signedDistance ) >= 1 )
			return 0;
		embed = true;
		t0 = 0;
		t1 = 1;
	}

	if ( t0 > t1 ) {
		float t = t0;
		t0 = t1;
		t1 = t;
	}
	//We never touch
	if ( t0 > 1.0f || t1 < 0.0f )
		return false;

	//Clamp
	if ( t0 < 0.0f ) t0 = 0.0f;
	if ( t1 > 1.0f ) t1 = 1.0f;

	//Check if we're on the inside of a triangle
	if ( !embed ) {
		Vec3 planeIntersection = sInfo->basePoint - triPlane.n + t0 * sInfo->velocity;
		if ( PointInTriangle( planeIntersection, a, b, c ) ) {
			sInfo->foundCollision = true;
			sInfo->eSpaceIntersection = planeIntersection;
			sInfo->eSpaceNearestDist = glm::length( sInfo->velocity ) * t0;
			sInfo->t = t0;
		}
	}


	//Sweeping
	//Vertices
	Vec3 vertices[3] = { a,b,c };

	for ( int i = 0; i < 3; i++ ) {
		float a = glm::dot( sInfo->velocity, sInfo->velocity );
		float b = 2 * glm::dot( sInfo->velocity, sInfo->basePoint - vertices[i] );
		float c = glm::length2( vertices[i] - sInfo->basePoint ) - 1;

		float x1 = 0;
		if ( SolveQuadratic( a, b, c, &x1 ) && ( x1 < 1.0f ) ) {
			float dist = glm::length( sInfo->velocity ) * x1;
			if ( !sInfo->foundCollision || dist < sInfo->eSpaceNearestDist ) {
				sInfo->foundCollision = true;
				sInfo->eSpaceIntersection = vertices[i];
				sInfo->eSpaceNearestDist = dist;
				sInfo->t = x1;
			}
		}
	}

	//Edges
	Vec3 edges[3] = {
		b - a,
		c - b,
		a - c };
	for ( int i = 0; i < 3; i++ ) {
		Vec3 edge = edges[i];
		Vec3 baseToVertex = vertices[i] - sInfo->basePoint;
		float edgeSquaredLength = glm::length2(edge);
		float edgeDotVelocity = glm::dot( edge, sInfo->velocity );
		float edgeDotBaseToVertex = glm::dot( edge, baseToVertex );
		float velocitySquaredLength = glm::length2( sInfo->velocity );
		float baseToVertexSquared = glm::length2( baseToVertex );
		// Calculate parameters for equation
		float a = edgeSquaredLength * -velocitySquaredLength +
			edgeDotVelocity * edgeDotVelocity;
		float b = edgeSquaredLength * ( 2 * glm::dot( sInfo->velocity, baseToVertex ) ) -
			2.0 * edgeDotVelocity * edgeDotBaseToVertex;
		float c = edgeSquaredLength * ( 1.0f - baseToVertexSquared ) +
			edgeDotBaseToVertex * edgeDotBaseToVertex;


		float x1 = 0;
		if ( !SolveQuadratic( a, b, c, &x1 ) ) {
			//Will not intersect with the infinite line
			continue;
		}
		if ( x1 > 1.0f ) continue;

		float f0 = ( ( edgeDotVelocity * x1 ) - ( edgeDotBaseToVertex ) ) / edgeSquaredLength;
		if ( f0 >= 0.0f && f0 <= 1.0f ) {
			if ( !sInfo->foundCollision || glm::length( sInfo->velocity ) * x1 < sInfo->eSpaceNearestDist ) {
				sInfo->foundCollision = true;
				sInfo->eSpaceIntersection = vertices[i] + edges[i] * f0;
				sInfo->eSpaceNearestDist = glm::length( sInfo->velocity ) * x1;
				sInfo->t = x1;
			}
		}
	}

	return sInfo->foundCollision;
}

bool BruteCastSphere( Vec3 pos, Vec3 velocity, Vec3 r, SweepInfo* outInfo ) {
	SweepInfo bestHit{};
	
	for ( int i = 0; i < physics.numBrushes; i++ ) {
		SweepInfo thisHit{};
		if ( CastSphere( pos, velocity, &physics.brushes[i], r, &thisHit ) ) {
			if ( !bestHit.foundCollision || bestHit.eSpaceNearestDist > thisHit.eSpaceNearestDist )
				bestHit = thisHit;
		}
	}

	if ( bestHit.foundCollision ) {
		*outInfo = bestHit;
		return true;
	}
	else
		return false;
}

bool CastSphere( Vec3 pos, Vec3 velocity, Brush* brush, Vec3 r, SweepInfo* outInfo ) {
	memset( outInfo, 0, sizeof( *outInfo ) );
	outInfo->r3Position = pos;
	outInfo->r3Velocity = velocity;
	outInfo->radius = r;
	outInfo->basePoint = EllipseFromWorld( pos, outInfo->radius );
	outInfo->velocity = EllipseFromWorld( outInfo->r3Velocity, outInfo->radius );
	outInfo->velocityNormalized = glm::normalize( outInfo->velocity );

	for ( int i = 0; i < brush->numPolygons; i++ ) {
		Polygon* pg = &brush->polygons[i];
		for ( int n = 0; n < pg->numTriangles; n ++ ) {
			SweepInfo info = *outInfo;
			BrushTri* tri = &pg->triangles[n];
			Vec3 a = brush->vertices[tri->v[0]];
			Vec3 b = brush->vertices[tri->v[1]];
			Vec3 c = brush->vertices[tri->v[2]];

			//DebugDrawSphere( a );
			//DebugDrawSphere( b );
			//DebugDrawSphere( c );


			//Only hit the front 
			//if ( glm::dot( velocity, pg->n ) > 0 )
			//	continue;

			if ( TestTriangleEllipse( &info, a, b, c ) ) {
				if ( !outInfo->foundCollision || info.eSpaceNearestDist < outInfo->eSpaceNearestDist) {
					*outInfo = info;
					outInfo->r3Norm = pg->n;
				}
			}
		} 
	}
	outInfo->r3Point = WorldFromEllipse( outInfo->eSpaceIntersection, Vec3( r ) );
	return outInfo->foundCollision;
}

bool AABBSweep( const BoundsMinMax& a, const BoundsMinMax& b, Vec3 velocity ) {
	BoundsMinMax ac = a;
	BoundsMinMax bc = b;
	//DebugDrawBoundsMinMax( &ac );
	//DebugDrawBoundsMinMax( &bc );

	if ( FastAABB( a, b ) ) return true;
	velocity = -velocity;
	float tMin = 0;
	float tMax = 1.0;

	for ( int i = 0; i < 3; i++ ) {
		if ( velocity[i] < 0.0f ) {
			if ( b.max[i] < a.min[i] ) return false;
			if ( a.max[i] < b.min[i] )
				tMin = glm::max( ( a.max[i] - b.min[i] ) / velocity[i], tMin );
			if ( b.max[i] > a.min[i] )
				tMax = glm::min( ( a.min[i] - b.max[i] ) / velocity[i], tMax );
		}
		if ( velocity[i] > 0.0f ) {
			if ( b.min[i] > a.max[i] ) return false;
			if ( b.max[i] < a.min[i] )
				tMin = glm::max( ( a.min[i] - b.max[i] ) / velocity[i], tMin );
			if ( a.max[i] > b.min[i] )
				tMax = glm::min( ( a.max[i] - b.min[i] ) / velocity[i], tMax );
		}
		if ( tMin > tMax ) 
			return false;
	}
	return true;
}

bool PhysicsQuerySweepStatic( Vec3 start, Vec3 velocity, Vec3 radius, SweepInfo* bestSweep ) {
	TEMP_ARENA_SET;
	memset( bestSweep, 0, sizeof( *bestSweep) );
	int aabbChecks = 0;

	int polyChecks = 0;

	BVHNode** stack = ( BVHNode** ) TEMP_ALLOC( sizeof( void* ) * physics.staticBVH.numNodes );
	int numStack = 1;
	stack[0] = &physics.staticBVH.nodes[physics.staticBVH.root];

	BoundsMinMax fastBounds{
		start - radius,
		start + radius
	};

	BoundsMinMax AABB2 {
		start - radius * 2.0f,
		start + velocity + radius * 2.0f
	};

	while ( numStack > 0 ) {
		BVHNode* node = stack[--numStack];
		aabbChecks++;

		//Note: Seems to be faster to just just quick AABB test rather than sweep and test
		// This means extra triangles but it seems to be fine
		//In the future I should look into
		//if ( !AABBSweep( fastBounds, node->bounds, velocity )  )
		if ( !FastAABB( AABB2, node->bounds ) )
			continue;

		//If leaf then get the actual object
		if ( node->isLeaf ) {
			//Collide with the map geometry (Already done if it's a leaf aabb)
			if ( node->object >= 0 ) {
				SweepInfo info{};
				polyChecks += physics.brushes[node->object].numPolygons;
				if ( CastSphere( start, velocity, &physics.brushes[node->object], radius, &info ) ) {
					if ( info.t < bestSweep->t || !bestSweep->foundCollision ) {
						*bestSweep = info;
					}
				}
			}
			else {
				assert( 0 );
			}
		}

		//If not just add children
		if ( node->child1 != -1 )
			stack[numStack++] = &physics.staticBVH.nodes[node->child1];
		if ( node->child2 != -1 )
			stack[numStack++] = &physics.staticBVH.nodes[node->child2];
	}
	return bestSweep->foundCollision;
}

bool PhysicsRaycastHull( Vec3 start, Vec3 dir, Brush* hull, HitInfo* info ) {
	memset( info, 0, sizeof( *info ) );
	float tmin = 0.0f;
	float tmax = 1.0f;

	for ( int i = 0; i < hull->numPolygons; i++ ) {
		Polygon* poly = &hull->polygons[i];
		//Intersect each plane
		Vec3 axis = glm::normalize( poly->n );
		float denom = glm::dot( poly->n, dir );
		float dist = -poly->d - glm::dot( poly->n, start );
		//Parallel
		if ( fabs( denom ) <= .005f ) {
			if ( -dist > 0 ) //??
				return 0;
		}
		else {
			float t = dist / denom;
			if ( denom < FEPSILON ) {
				if ( t > tmin ) tmin = t;
			}
			else {
				if ( t < tmax ) tmax = t;
			}
			if ( tmin > tmax )
				return 0;
		}
	}
	info->didHit = true;
	info->point = start + dir * tmin;
	info->dist = tmin;
	return 1;
}

bool RaycastAABB( const Vec3& start, const Vec3& velocity, const BoundsHalfWidth& aabb, HitInfo* info ) {
	Vec3 min = aabb.center - aabb.width;
	Vec3 max = aabb.center + aabb.width;

	float t1 = ( min.x - start.x ) / velocity.x;
	float t2 = ( max.x - start.x ) / velocity.x;
	float t3 = ( min.y - start.y ) / velocity.y;
	float t4 = ( max.y - start.y ) / velocity.y;
	float t5 = ( min.z - start.z ) / velocity.z;
	float t6 = ( max.z - start.z ) / velocity.z;

	float tmin = glm::max( glm::max( glm::min( t1, t2 ), glm::min( t3, t4 ) ), glm::min( t5, t6 ) );
	float tmax = glm::min( glm::min( glm::max( t1, t2 ), glm::max( t3, t4 ) ), glm::max( t5, t6 ) );

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if ( tmax < 0 ) {
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if ( tmin > tmax ) {
		return false;
	}

	//Inside of box. Rays should nto count inside of an object.
	if ( tmin < 0.f ) {
		return false;
	}
	
	info->dist = tmin;
	info->didHit = true;
	return tmin;

}

bool PhysicsQueryRaycast( Vec3 start, Vec3 velocity, HitInfo* best ) {
	memset( best, 0, sizeof( *best ) );

	//First Check Static Geometry
	//TODO BVH
	HitInfo bestStatic{};
	for ( int i = 0; i < physics.numBrushes; i++ ) {
		HitInfo thisInfo{};
		if ( PhysicsRaycastHull( start, velocity, &physics.brushes[i], &thisInfo ) ) {
			if ( thisInfo.didHit < bestStatic.dist ) {
				bestStatic = thisInfo;
			}
		}
	}
	
	HitInfo bestDynamic{};
	if ( PhysicsRaycastDynamic( start, velocity, &bestDynamic ) )
		*best = bestDynamic;
	
	//if ( bestStatic.didHit && bestStatic.dist < bestDynamic.dist )
	//	*best = bestStatic;

	if ( best->didHit )
		best->point = start + velocity * best->dist;

	return best->didHit;
}

bool PhysicsRaycastDynamic( Vec3 start, Vec3 velocity, HitInfo* out ) {
	HitInfo best{};

	memset( out, 0, sizeof( *out) );
	for ( int i = 0; i < physics.numActiveColliders; i++ ) {
		HitInfo thisHit{};
		BoundsHalfWidth bounds;
		CharacterCollider* collider = physics.activeColliders[i];
		bounds.center = collider->offset + collider->bounds.center;
		bounds.width = collider->bounds.width;

		if ( RaycastAABB( start, velocity, bounds, &thisHit ) ) {
			if ( thisHit.dist < best.dist || !best.didHit ) {
				thisHit.entity = collider->owner;
				best = thisHit;
			}
		}
	}

	*out = best;
	return best.didHit;
}

bool PhysicsQueryIntersectEntities( CharacterCollider* cc, EntityCollisonQuery* outQuery ) {
	memset( outQuery, 0, sizeof( outQuery ) );
		
	for ( int i = 0; i < physics.numActiveColliders; i++ ) {
		//Never Query self
		if ( cc->owner == physics.activeColliders[i]->owner ) {
			continue;
		}

		CharacterCollider* b = physics.activeColliders[i];

		BoundsMinMax boxA = { 
			cc->offset + cc->bounds.center - cc->bounds.width, 
			cc->offset + cc->bounds.center + cc->bounds.width 
		};

		BoundsMinMax boxB = {
			b->offset + b->bounds.center - b->bounds.width,
			b->offset + b->bounds.center + b->bounds.width
		};

		if ( FastAABB( boxA,boxB ) ) {
			outQuery->didHit = true;
			outQuery->entity = b->owner;
			return true;
		}
	}
	return false;
}

void PhysicsRigidBodiesUpdate() {
	for ( int i = 0; i < MAX_RIGIDBODIES; i++ ) {
		RigidBody* body = &physics.rigidBodies[i];
		
		if ( body->removeTime < gameTime )
			body->state = RB_NONE;

		if ( body->state != RB_IN_MOTION )
			continue;
		
		Vec3 velLast = body->velocity;
		//Gravity
		body->velocity -= 10 * dt;
		Vec3 frameVel = ( velLast + body->velocity ) * 0.5f * dt;

		if ( body->emitter )
			body->emitter->pos = body->pos;

		SweepInfo info;
		//todo handle collsions properly
		if ( BruteCastSphere( body->pos, frameVel, Vec3( body->radius ), &info ) ) {
			body->pos = info.r3Point;
			body->state = RB_STATIC;

			//Remove emitter, it shoots downwards
			if ( body->emitter )
				body->emitter->currentTime = body->emitter->lifeTime + 1.0f;
		}
		else {
			body->pos += frameVel;
		}
	}
}

RigidBody* NewRigidBody() {
	for ( int i = 0; i < MAX_RIGIDBODIES; i++ ) {
		RigidBody* body = &physics.rigidBodies[i];
		if ( body->state == ACTIVE_INACTIVE ) {
			memset( body, 0, sizeof( *body ) );
			body->modelScale = 1.0f;
			body->state = RB_IN_MOTION;
			physics.numRigidBodies++;
			return body;
		}
	}
	return 0;
}

void CreateBoid( Entity* entity ) {
	physics.boids[physics.numBoids++] = entity;
}

void RemoveBoid( Entity* entity ) {
	for ( int i = 0; i < MAX_ENTITIES; i++ ) {
		if ( physics.boids[i] == entity )
			physics.boids[i] = physics.boids[--physics.numBoids];
	}
}

#define BOID_MAX_RANGE 20.0f
void UpdateBoids() {
	TEMP_ARENA_SET;


}