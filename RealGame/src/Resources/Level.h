#pragma once 
#include "def.h"
#include <vector>

struct Plane {
	Vec3 n;
	float d;
};

class Level {
public:
	ScratchArena arena;
	
	char path[MAX_PATH_LENGTH];
	int numBrushes;
	int numVertices;
	int numIndices;
	int numFaces;
};

void CreateLevel( Level* level, void* memory, u32 size );

//Note All loading from here (RenderLoadLevel/PhysicsLoadLevel) uses the level.scratch arena
//This way 1 "clear" gets rid of all the memory.
//Others still have to adjust values to show they arent using any of the memory
bool LoadLevel( Level* level, const char* path );
void UnloadLevel(Level* level);
void ChangeLevel();
void ConsoleChangeLevel();