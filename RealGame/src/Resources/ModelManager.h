#pragma once
#include "def.h"
#include "renderer/GLBuffer.h"
#include "Renderer/Renderer.h"
#include "Animation.h"
//Note: mesh can only have 1 material

//Models
struct Node {
	Vec3 t;
	Quat r;
	Vec3 s;

	int boneID;//Bone index. -1 = no bone
	int index; //Node index
	int parent;
	int numChildren;
	Node** children;
	char name[32];
};


struct DrawVertex {
	Vec3 pos;
	Vec3 normal;
	Vec2 tex;
	Vec2 lightmapTex;
	Vec4 tangents;
	IVec4 bones;
	Vec4 weights;
};


class Mesh {
public:
	char name[MAX_NAME_LENGTH];
	struct Texture* texture;
	u32 numVertices;
	u32 numIndices;
	GLBuffer buffer;
};

class Model {
public:
	char path[MAX_PATH_LENGTH];
	u32 numMeshes;
	u32 numAnimations;
	Mesh* meshes;
	struct Skeleton* skeleton;
	struct AnimationClip** animations;
};

struct ModelInfo {
	ModelInfo* next;
	//Actual Model
	Model model; 
	//Add an arena so we can add things like joints from the chunk
	ScratchArena arena;
};

class ModelManager {
public:
	PoolArena modelArena;
	ModelInfo* modelHead;
	
	PoolArena animationArena;
	struct AnimationClipInfo* animHead;
};
extern ModelManager modelManager;
	
void CreateModelManager( ModelManager* manager, u32 memorySize, void* memory, u32 animationSize, void* animMemory );
Model* ModelManagerAllocate( ModelManager* manager, const char* path );
Model* ModelManagerGetModel( const char* path , bool warnOnFail);
AnimationClip* ModelFindAnimation( Model* model, const char* animation );