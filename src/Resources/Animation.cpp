#include "ModelManager.h"

float GetScaleFactor( float lastTimeStamp, float nextTimeStamp, float animationTime ) {
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;

	if ( scaleFactor > 1.0f )
		printf( "" );
	return scaleFactor;
}

void AnimatePoseNoAnimation( SkeletonPose* pose ) {
	for ( int i = 0; i < pose->skeleton->numBones; i++ ) {
		pose->pose[i].t = pose->skeleton->joints[i].t;
		pose->pose[i].r = pose->skeleton->joints[i].r;
	}
}

int KeyFrameIndex( float t, float* keyFrames, int length) {
	assert( length > 0 );
	for ( int i = 0; i < length; i++ ) {
		if ( keyFrames[i + 1] > t )
			return i;
	}
	return 0;
}

Vec3 GetTranslation( float time, AnimationClip* clip, int bone ) {
	JointKeyFrames* frames = &clip->jointKeyFrames[bone];

	if ( frames->numPosKeys == 0 )
		return clip->skeleton->joints[bone].t;

	if ( frames->numPosKeys == 1 )
		return frames->posKeys[0];

	int p0 = KeyFrameIndex( time, frames->posTimes, frames->numPosKeys );
	int p1 = p0 + 1;
	float scale = GetScaleFactor( frames->posTimes[p0], frames->posTimes[p1], time );
	return glm::mix( frames->posKeys[p0], frames->posKeys[p1], 0.0f );
}

Quat GetRotation( float time, AnimationClip* clip, int bone ) {
	JointKeyFrames* frames = &clip->jointKeyFrames[bone];

	if ( frames->numRotKeys == 0 )
		return clip->skeleton->joints[bone].r;

	if ( frames->numRotKeys == 1 )
		return frames->rotKeys[0];

	int p0 = KeyFrameIndex( time, frames->rotTimes, frames->numRotKeys );
	int p1 = p0 + 1;
	float scale = GetScaleFactor( frames->rotTimes[p0], frames->rotTimes[p1], time );
	return glm::slerp( frames->rotKeys[p0], frames->rotKeys[p1], 0.0f );
}

inline bool UniformScale( const Vec3& v ) {
	return( fabs( v.x - v.y ) < .1f && fabs( v.x - v.z ) < .1f );
	//return ( v.x == v.y && v.x == v.z );
}
float GetScale( float time, AnimationClip* clip, int bone ) {
	JointKeyFrames* frames = &clip->jointKeyFrames[bone];

	if ( frames->numScaleKeys == 0 ) {
		Vec3 scale = clip->skeleton->joints[bone].s;
		assert( UniformScale( scale ) );
		return scale.x;
	}

	if ( frames->numScaleKeys == 1 ) {
		Vec3 scale = frames->scaleKeys[0];
		assert( UniformScale( scale ) );
		return scale.x;
	}

	int p0 = KeyFrameIndex( time, frames->scaleTimes, frames->numScaleKeys );
	int p1 = p0 + 1;
	float scale = GetScaleFactor( frames->scaleTimes[p0], frames->scaleTimes[p1], time );
	Vec3 scaleFinal = glm::mix( frames->scaleKeys[p0], frames->scaleKeys[p1], 0.0f );
	assert( UniformScale( scaleFinal ) );
	return scaleFinal.x;
}


void AnimatePose( float time, AnimationClip* clip, SkeletonPose* pose ) {
	if ( !clip ) {
		AnimatePoseNoAnimation( pose );
		return;
	}

	assert( clip->skeleton == pose->skeleton );

	for ( int i = 0; i < clip->skeleton->numNodes; i++ ) {
		pose->pose[i].t = GetTranslation( time, clip, i );
		pose->pose[i].r = GetRotation( time, clip, i );
		pose->pose[i].s = GetScale( time, clip, i );
	}
}

void UpdatePose( int index, Mat4 prev, SkeletonPose* pose ) {
	Skeleton* skeleton = pose->skeleton;

	Node* node = &skeleton->joints[index];
	JointPose* jointPose = &pose->pose[index];

	Mat4 t = glm::translate( Mat4( 1.0 ), jointPose->t );
	Mat4 r = glm::toMat4( jointPose->r );
	Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( jointPose->s ) );

	Mat4 local = t * r * s;
	Mat4 global = prev * local;

	if ( node->boneID != -1 ) {
		pose->globalPose[node->boneID] = global * pose->skeleton->inverseBinds[node->boneID];// *node->invBind;
	}

	for ( int i = 0; i < node->numChildren; i++ ) {
		UpdatePose( node->children[i]->index, global, pose );
	}
}
