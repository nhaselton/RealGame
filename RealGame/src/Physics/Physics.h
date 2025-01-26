#pragma once
#include "def.h"

struct CharacterCollider;

struct BrushTri {
	u32 v[3];
};

struct Polygon {
	Vec3 n;
	float d;

	BrushTri* triangles;
	u32 numTriangles;
};

struct Brush {
	Vec3* vertices;
	Polygon* polygons;
	u32 numVertices;
	u32 numPolygons;
};

struct Physics {
	Brush* brushes;
	int numBrushes;
};
extern Physics physics;

struct SweepInfo {
	Vec3 radius;

	//R3 Space
	Vec3 r3Velocity;
	Vec3 r3Position;

	//eSpace
	Vec3 velocity;
	Vec3 velocityNormalized;
	Vec3 basePoint;

	//Hit Info
	bool foundCollision;
	float eSpaceNearestDist;
	Vec3 eSpaceIntersection;

	Vec3 r3Point;
	Vec3 r3Norm;
	float t;
};


struct HitInfo {
	bool didHit;
	Vec3 point;
	Vec3 normal;
	//t [0,1] for cast.
	//Overlap in direction of normal for collisions
	float dist;
};



void PhysicsInit();
void PhysicsLoadLevel( struct Level* level, struct NFile* file );

//
bool BruteCastSphere( Vec3 pos, Vec3 velocity, Vec3 r, SweepInfo* outInfo );
bool CastSphere( Vec3 pos, Vec3 velocity, Brush* brush, Vec3 r, SweepInfo* outInfo );
Vec3 MoveAndSlide( CharacterCollider* characterController, Vec3 velocity, int bounces = 3 );

Vec3 EllipseFromWorld( const Vec3& point, const Vec3& radius );
Vec3 WorldFromEllipse( const Vec3& point, const Vec3& radius );
