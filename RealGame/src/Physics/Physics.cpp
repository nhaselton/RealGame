#include "Physics.h"
#include "Resources\Level.h"
#include "Renderer/DebugRenderer.h"
void PhysicsInit() {

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
	int a = 0;
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

bool BruteCastSphere( Vec3 pos, Vec3 velocity, float r, SweepInfo* outInfo ) {
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

bool CastSphere( Vec3 pos, Vec3 velocity, Brush* brush, float r, SweepInfo* outInfo ) {
	memset( outInfo, 0, sizeof( *outInfo ) );
	outInfo->r3Position = pos;
	outInfo->r3Velocity = velocity;
	outInfo->radius = Vec3( r );
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

#if 0

float signedDistance = glm::dot( polygon->n, pos ) - polygon->d;
float NdotV = glm::dot( velocity, polygon->n );

float t0 = ( 1 - signedDistance ) / NdotV;
float t1 = ( -1 - signedDistance ) / NdotV;

bool embed = false;
if ( NdotV == 0.0f ) {
	if ( fabs( signedDistance ) >= r )
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
	Vec3 planeIntersection = pos - polygon->n + t0 * velocity;
	if ( PointInTriangle( planeIntersection, a, b, c ) ) {
		info->didHit = true;
		info->dist = t0;
		info->point = pos + velocity * t0;
		info->normal = polygon->n;
		return true;
	}
}
//Sweeping
return false;
//Vertices
Vec3 vertices[3] = { a,b,c };

for ( int i = 0; i < 3; i++ ) {
	float a = glm::dot( velocity, velocity );
	float b = 2 * glm::dot( velocity, pos - vertices[i] );
	float c = glm::length2( vertices[i] - pos ) - 1;

	float x1 = 0;
	if ( SolveQuadratic( a, b, c, &x1 ) ) {
		float dist = x1;
		if ( !info->didHit || dist < info->dist ) {
			//This Might be wrong. It should probably be veritces[i] - normal * Radius becuase right now its center is the same spot as the vertex
			info->didHit = true;
			info->dist = dist;
			info->normal = pos - vertices[i];
			info->point = pos + velocity * info->dist;//vertices[i];
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
	Vec3 baseToVertex = vertices[i] - pos;
	float edgeSquaredLength = glm::length2( edge );
	float edgeDotVelocity = glm::dot( edge, velocity );
	float edgeDotBaseToVertex = glm::dot( edge, baseToVertex );
	float velocitySquaredLength = glm::length2( velocity );
	float baseToVertexSquared = glm::length2( baseToVertex );
	// Calculate parameters for equation
	float a = edgeSquaredLength * -velocitySquaredLength +
		edgeDotVelocity * edgeDotVelocity;
	float b = edgeSquaredLength * ( 2 * glm::dot( velocity, baseToVertex ) ) -
		2.0 * edgeDotVelocity * edgeDotBaseToVertex;
	float c = edgeSquaredLength * ( 1.0f - baseToVertexSquared ) +
		edgeDotBaseToVertex * edgeDotBaseToVertex;


	float x1 = 0;
	if ( !SolveQuadratic( a, b, c, &x1 ) ) {
		//Will not intersect with the infinite line
		continue;
	}

	float f0 = ( ( edgeDotVelocity * x1 ) - ( edgeDotBaseToVertex ) ) / edgeSquaredLength;
	if ( f0 >= 0.0f && f0 <= 1.0f ) {
		if ( !info->didHit || x1 < info->dist ) {
			info->dist = x1;
			info->point = pos + velocity * x1;//vertices[i] + f0 * edges[i];
			//Note: Maybe do perpendicular to edge? Probably safter to just do poly normal
			info->normal = polygon->n;
			info->didHit = true;
		}
	}
	}

return info->didHit;
}

bool CastSphere( Vec3 pos, float r, Vec3 velocity, HitInfo* outInfo ) {
	memset( outInfo, 0, sizeof( *outInfo ) );

	for ( int i = 0; i < 6; i++ ) {
		Polygon* pg = &physics.brush.p[i];
		for ( int n = 0; n < 6; n += 3 ) {
			HitInfo info{};

			Vec3 a = pg->verts[n + 0];
			Vec3 b = pg->verts[n + 1];
			Vec3 c = pg->verts[n + 2];

			//DebugDrawSphere( a );
			//DebugDrawSphere( b );
			//DebugDrawSphere( c );

			//Only hit the front 
			if ( glm::dot( velocity, pg->n ) > 0 )
				continue;


			if ( TestTriangle( pos, r, velocity, pg, a, b, c, &info ) ) {
				if ( !outInfo->didHit || info.dist < outInfo->dist ) {
					*outInfo = info;
				}
				DebugDrawSphere( info.point, 1, Vec3( 0, 0, 1 ) );
				//DebugDrawLine( info.point, info.point + info.normal * 1.2f);
			}
		}
	}
	return outInfo->didHit;
}
#endif
