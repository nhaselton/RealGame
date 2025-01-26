#pragma once
#include "CumDef.h"

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