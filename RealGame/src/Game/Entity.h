#pragma once
#include "def.h"
#include "physics/Colliders.h"

struct BoundsHalfWidth;
struct SkeletonPose;

enum entityType_t {
	ENT_NONE,
	ENT_PLAYER,
	ENT_OGRE,
	ENT_GOBLIN,
	ENT_WIZARD,

};

enum activeState_t {
	ACTIVE_INACTIVE = 0,
	ACTIVE_WAIT_FOR_REMOVE = 1, //removed at end of frame
	ACTIVE_ACTIVE = 2,
};

enum trigger_t {
	TRIGGER_NONE = 0,
	TRIGGER_PRINT_MESSAGE = 1,
	TRIGGER_START_ENCOUNTER = 2,
	TRIGGER_SPAWN_SINGLE_AI = 4,
	TRIGGER_SPAWN_MULTI_AI = 8
};

struct Trigger {
	trigger_t type;
	char willTrigger[MAX_PATH_LENGTH];
	char myName[MAX_PATH_LENGTH];
	BoundsMinMax bounds;
};

struct RenderModel {
	class Model* model;
	SkeletonPose* pose;
	//todo pose

	Vec3 translation;
	Quat rotation;
	Vec3 scale;
};
//Projectiles are completely separated from entities becuase they share no functionality besides the charactercollider
//In ray casts, shape casts, etc. the projectile is never queried against 
// (TODO how to handle few exceptions)
class Projectile {
public:
	activeState_t state;
	//I do want it to sweep
	Vec3 velocity;
	class Entity* owner;

	float speed;
	CharacterCollider collider;
	void ( *OnCollision ) ( Projectile* projectile, Entity* entity );
	float spawnTime;

	RenderModel model;

	//Linear Functions
	bool isLinear;
	float staticImpactTime;
	//todo float impactTime
	//hitCallback?
	//Damage?
	//RenderModel
};

struct State {
	void ( *OnEnter ) ( class Entity* entity );
	void ( *OnUpdate ) ( class Entity* entity );
	void ( *OnExit ) ( class Entity* entity );
};

ENT_CLASS Entity {
public:
	EVAR Vec3 pos ENT_RENAME( "origin" );
	Quat rotation;

	Vec3 target;

	Vec3 boidVelocity;

	CharacterCollider* bounds;
	RenderModel* renderModel;

	struct AnimationClip* currentAnimation;
	float currentAnimationTime; 
	float currentAnimationPercent; //t = [0,1]
	float animTimeScale;//how fast should the anims be
	float lastAnimationTime;

	u32 state;

	EVAR int health;
	EVAR int maxHealth;

	char spawnTag[MAX_TAG_LENGTH];
	class Encounter* encounter;

	void ( *Update ) ( class Entity* entity );
	void ( *OnHit ) ( struct EntityHitInfo info );
	void ( *RecievedAnimationEvent ) ( class Entity* entity, struct AnimationEvent* event );
};
//Init
void EntityGenerateRenderModel( Entity* entity, class Model* model, ScratchArena* arena );
//Animation
void EntityStartAnimation( Entity* entity, int index );
void EntityAnimationUpdate( Entity* entity, float dt );
//Runtime
void EntityMove( Entity* entity, Vec3 velocity );
void EntityLookAtPlayer( Entity* entity );

inline Vec3 EntityForward( Entity* entity ) {
	return glm::normalize(entity->rotation * Vec4( 0, 0, 1, 0 ));
}

struct Model* DefLoadModel( const char* path );