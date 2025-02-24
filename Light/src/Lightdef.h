#pragma once
#include <glm/glm.hpp>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

//How often is it sampled
#define ATLAS_SIZE ( 1024.0f )
#define TEXEL_SIZE_WORLD_UNITS (1.0f) 
#define AMBIENT (.15f)
#define PAD (0)

struct LightSurface {
	//World Space
	Vec3 n;
	float d;
	Vec3 axes[3];//u,v,texNormal
	Vec3 texToWorld[2];
	Vec3 texOrigin;

	//Texture info
	//.x = u, .y = v
	Vec2 realMin;
	Vec2 realMax;
	Vec2 paddedMin;
	Vec2 paddedMax;
	Vec2 paddedTextureSize;

	struct AtlasNode* node;
};

struct LightMapFace {
	Vec3 normal;
	float d;
	Vec3 u;
	Vec3 v;
	Vec3 texNormal;
	int firstVertex;
	int numVertices;
	int firstIndex;
	int numIndices;
};

struct AtlasNode {
	//Min max on the atlas
	Vec2 min;
	Vec2 max;
	//Face Vertices min max
	Vec3 worldMin;
	Vec3 worldMax;
	int faceIndex;
	IVec2 texels;
	struct AtlasNode* left;
	struct AtlasNode* right;

	Vec3 vertMin;
	Vec3 vertMax;
};

struct Atlas {
	AtlasNode* head;
	int count;
};

struct LightMapBrush {
	u32 firstFace;
	u32 numFaces;
};
