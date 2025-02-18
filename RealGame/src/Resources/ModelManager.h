#pragma once
#include "def.h"
#include "renderer/GLBuffer.h"
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

struct JointPose {
	Vec3 t;
	Quat r;
	float s; //Assume always uniform scale
};

struct DrawVertex {
	Vec3 pos;
	Vec3 normal;
	Vec2 tex;
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

// Animations

class AnimationManager {
	PoolArena animationArena;
	struct AnimationInfoClip* head;
};

struct Skeleton {
	char name[MAX_NAME_LENGTH];
	u32 numBones; //This is how many get sent to the GPU
	u32 numNodes; // This is how many get updated in the tree

	int root;

	Node* joints;
	Node** bones; //List of bones in correcet boneIndex order
	Mat4* inverseBinds; //There are numBones many, and ecah boneID corrosponds to one of thsee
};
struct SkeletonPose {
	struct Skeleton* skeleton;
	JointPose* pose;
	Mat4* globalPose;
};

#if 0
struct PositionKey {
	Vec3 p;
	float t;
};

struct RotationKey {
	Quat r;
	float t;
};
#endif

struct JointKeyFrames {
#if 0
	PositionKey* posKeys;
	RotationKey* rotKeys;
#endif
	Vec3* posKeys;
	float* posTimes;

	Quat* rotKeys;
	float* rotTimes;

	Vec3* scaleKeys;
	float* scaleTimes;

	int numPosKeys;
	int numRotKeys;
	int numScaleKeys;
};

struct AnimationClip {
	char name[MAX_PATH_LENGTH];
	struct Skeleton* skeleton;

	float duration;
	bool looping;

	JointKeyFrames* jointKeyFrames;
	int numKeyframes;
};

//Like modelInfo, this is given extra memory that can scratch alloced
struct AnimationClipInfo {
	AnimationClipInfo* next;
	AnimationClip animation;
	ScratchArena arena;
};



inline Vec3 BlendTranslation( const Vec3& a, const Vec3& b, float t ) {
	return glm::mix( a, b, t );
}

inline Quat BlendRotation( const Quat& a, const Quat& b, float t ) {
	return glm::slerp( a, b, t );
}

void CreateModelManager( ModelManager* manager, u32 memorySize, void* memory, u32 animationSize, void* animMemory );
Model* ModelManagerAllocate( ModelManager* manager, const char* path );

float GetScaleFactor( float lastTimeStamp, float nextTimeStamp, float animationTime );
void AnimatePoseNoAnimation( SkeletonPose* pose );
Quat GetRotation( float time, AnimationClip* clip, int bone );
void AnimatePose( float time, AnimationClip* clip, SkeletonPose* pose );
void UpdatePose( int index, Mat4 prev, SkeletonPose* pose );
Model* ModelManagerGetModel( const char* path );