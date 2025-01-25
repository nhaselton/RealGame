#pragma once
#include "def.h"
struct DPlane {
	dVec3 n;
	double d;
};
struct DBrushVertex {
	dVec3 pos;
	dVec3 normal;
	dVec2 uv;
};