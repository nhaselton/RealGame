#pragma once
#include "def.h"

struct Node;

struct JointPose {
	Vec3 t;
	Quat r;
	float s; //Assume always uniform scale
};

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

struct JointKeyFrames {
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

//These are gameplay related events 

enum animationEvent_t {
	ANIM_EVENT_HIT_ACTIVE,
	ANIM_EVENT_HIT_END,
	ANIM_EVENT_SHOOT_PROJECTILE,
};

struct AnimationEvent {
	animationEvent_t type;
	float time;
};

struct AnimationClip {
	char name[MAX_PATH_LENGTH];
	struct Skeleton* skeleton;

	float duration;
	bool looping;

	JointKeyFrames* jointKeyFrames;
	int numKeyframes;

	AnimationEvent* events;
	int numEvents;
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

float GetScaleFactor( float lastTimeStamp, float nextTimeStamp, float animationTime );
void AnimatePoseNoAnimation( SkeletonPose* pose );
Quat GetRotation( float time, AnimationClip* clip, int bone );
void AnimatePose( float time, AnimationClip* clip, SkeletonPose* pose );
void UpdatePose( int index, Mat4 prev, SkeletonPose* pose );