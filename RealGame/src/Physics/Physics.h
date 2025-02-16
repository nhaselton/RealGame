#pragma once
#include "def.h"
#include "Colliders.h"

struct BrushTri {
	u32 v[3];
};

struct Polygon {
	Vec3 n;
	float d;

	BrushTri* triangles;
	u32 numTriangles;
};

struct EntityCollisonQuery {
	Entity* entity;
	bool didHit;
};

struct Brush {
	Vec3* vertices;
	Polygon* polygons;
	u32 numVertices;
	u32 numPolygons;
};

struct BVHNode {
	BoundsMinMax bounds;
	int nodeIndex;
	//if leaf node: index into physics.brushes[]
	int object;
	int parent;
	int child1;
	int child2;
	bool isLeaf;
};

struct BVHTree {
	BVHNode* nodes;
	int numNodes;
	int root;
};

enum rigidBodyState_t {
	RB_NONE,
	RB_IN_MOTION,
	RB_STATIC,
	RB_CAN_REMOVE
};

//These are never queried by anything
//its just for gibs, dead bodies, etc.
struct RigidBody {
	rigidBodyState_t state;
	Vec3 pos;
	Vec3 velocity;
	float radius; //Faster to collide with surface. Can also sweep
	float removeTime; //What gametime should this be removed at

	//For Drawing
	class Model* model;
	Vec3 visualOffset;
	float modelScale;
	class ParticleEmitter2* emitter;
	struct SkeletonPose* pose; //CAN be null
};


struct Physics {
	Brush* brushes;
	int numBrushes;
	BVHTree staticBVH;
	
	//This is exclusively for entities
	CharacterCollider entityColliders[MAX_ENTITIES];

	//This may change in the future, should be anythign dynamic that raycasts/entitys care about
	//If shootable projectile, put in here. May have to expand capacity to more than MAX_ENTITIES then
	CharacterCollider* activeColliders[MAX_ENTITIES];
	int numActiveColliders;

	//Enemys that use boid style movement
	Entity* boids[MAX_ENTITIES];
	int numBoids;

	RigidBody rigidBodies[MAX_RIGIDBODIES];
	int numRigidBodies;
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
	//In case of raycasting dynamics
	Entity* entity;
};

void PhysicsInit();
void PhysicsLoadLevel( struct Level* level, struct NFile* file );
void PhysicsUnloadLevel();

void PhysicsRigidBodiesUpdate();

bool PhysicsQueryRaycast( Vec3 start, Vec3 velocity, HitInfo* best );
bool PhysicsQuerySweepStatic( Vec3 start, Vec3 velocity, Vec3 radius, SweepInfo* bestSweep );
bool PhysicsQuerySweepStatic2( Vec3 start, Vec3 velocity, Vec3 radius, SweepInfo* bestSweep );
bool PhysicsQueryIntersectEntities( CharacterCollider* cc, EntityCollisonQuery* outQuery );

void CreateBoid( class Entity* entity );
void RemoveBoid( class Entity* entity );
void UpdateBoids();

bool PhysicsRaycastDynamic( Vec3 start, Vec3 velocity, HitInfo* info );
bool PhysicsRaycastStaticFast( Vec3 start, Vec3 velocity );
bool BruteCastSphere( Vec3 pos, Vec3 velocity, Vec3 r, SweepInfo* outInfo );
bool CastSphere( Vec3 pos, Vec3 velocity, Brush* brush, Vec3 r, SweepInfo* outInfo );
//Should the characterController offset be moved to the new position
//Returns where the entity.position/offset should be no matter what
Vec3 MoveAndSlide( CharacterCollider* characterController, Vec3 velocity, int bounces = 3, bool adjustCharacterController = false );

Vec3 EllipseFromWorld( const Vec3& point, const Vec3& radius );
Vec3 WorldFromEllipse( const Vec3& point, const Vec3& radius );

bool PhysicsRaycastHull( Vec3 start, Vec3 dir, Brush* hull, HitInfo* info );

RigidBody* NewRigidBody();

inline bool FastAABB( const BoundsMinMax& a, const BoundsMinMax& b ) {
	return
		( a.min.x < b.max.x && a.max.x > b.min.x &&
			a.min.y < b.max.y && a.max.y > b.min.y &&
			a.min.z < b.max.z && a.max.z > b.min.z );
}
