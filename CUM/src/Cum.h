#pragma once
#include "Physics\Colliders.h"

struct DPlane {
	dVec3 n;
	double d;
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

struct LightmapInfo {
	u32 numBrushes;
	u32 numFaces;
	u32 numVertices;
	u32 numIndices;

	u32* facesPerBrush;
	struct LightMapFace* faces;
	struct DrawVertex* drawVertices;
	u32* indices;
};

struct LightMapBrush {
	u32 firstFace;
	u32 numFaces;
};

struct DBrushVertex {
	dVec3 pos;
	dVec3 normal;
	dVec2 uv;
	dVec2 lightmapUV;
};

struct NPFace {
	//
	u32 textureIndex;
	Vec4 texU;
	Vec4 texV;
	Vec3 info;//Rotation ScaleU, ScaleV

	//Plane
	dVec3 n;
	float d;

	//Proccessed Info
	u32 firstVertex;
	u32 numVertices;
	u32 firstIndex;
	u32 numIndices;
};

struct NPBrush {
	u32 firstPlane;
	int numFaces;
	u32 firstVertex;
	u32 numVertices;
	u32 firstIndex;
	u32 numIndices;
	BoundsMinMax bounds;
};

extern double scale;
extern bool fixNormals;
bool Compile( const char* input, const char* output);
bool LoadWorldSpawn( class Parser* parser, const char* output );
