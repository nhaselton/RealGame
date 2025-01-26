#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "def.h"

class Shader;

#define MAX_PRIMS 150000
#define MAX_LINES_BATCH 1000
#define NUM_LINE_BATCHES 4 //how many different sized lines can you have

enum DebugPrimType {
	DP_AABB,
	DP_SPHERE,
	DP_LINE,
};

struct DebugPrim {
	struct AABB {
		DebugPrimType type;
		Vec3 center;
		Vec3 size;
	};

	struct Sphere {
		Vec3 center;
		Vec3 radius; //V3 For Ellipse
	};

	struct Line {
		Vec3 a;
		Vec3 b;
	};

	union Data {
		AABB aabb;
		Sphere sphere;
		Line line;
	};

	enum flags : u8 {
		DPF_WIREFRAME = 0b1,
		DPF_DEPTH_TEST = 0b11
	};
	
	DebugPrimType type;
	Data data;
	Vec3 color;

	u8 flags;
	u8 lineWidth; //User puts in a float. Program will do thickness *= .1

	float duration;
	DebugPrim* next;
};

struct DebugLine {
	Vec3 a;
	Vec3 colora;
	Vec3 b;
	Vec3 colorb;
};


//This is debug stuff, just keep it simple no classes and use static global vars in .cpp
struct DebugRenderer {
	int numPrims;
	PoolArena primPool;
	DebugPrim* prims;
};

void CreateDebugRenderer( class Renderer* renderer, void* memory, u32 size );
void DebugRendererFrame( Mat4 view, Mat4 projection, float dt );
 
void DebugDrawSphere( Vec3 center = Vec3( 0 ), float radius = 1, Vec3 color = Vec3( 0, 1, 0 ), bool wireframe = true, float linewidth = 1, bool depthTest = true, float duration = 0.0f );
void DebugDrawAABB( Vec3 center = Vec3( 0 ), Vec3 bounds = Vec3( 1 ), float duration = 0.0f, Vec3 color = Vec3( 0, 1, 0 ), bool wireframe = true, float linewidth = 1, bool depthTest = true );
void DebugDrawBoundsMinMax( struct BoundsMinMax* bounds, Vec3 color = Vec3( 0, 1, 0 ), float duration = 0.0f, bool wireframe = true, float linewidth = 1, bool depthTest = true );
void DebugDrawBoundsHalfWidth( struct BoundsHalfWidth* bounds, Vec3 color = Vec3( 0, 1, 0 ), float duration = 0.0f, bool wireframe = true, float linewidth = 1, bool depthTest = true );
void DebugDrawLine( Vec3 a = Vec3( 0 ), Vec3 b = Vec3( 1 ), Vec3 color = Vec3( 0, 1, 0 ), float linewidth = 1, bool depthTest = true, bool screenSpace = false, float duration = 0.0f );
void DebugDrawEllipse( Vec3 center, Vec3 radius = Vec3(1), Vec3 color = Vec3(0, 1, 0), bool wireframe = true, bool depthTest = true, bool screenSpace = false, float duration = 0.0f);
void DebugDrawCharacterCollider( struct CharacterCollider* collider, Vec3 color = Vec3( 0, 1, 0 ), bool wireframe = true, bool depthTest = true, float duration = 0.0f );