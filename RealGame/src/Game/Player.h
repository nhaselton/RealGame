#pragma once
#include "def.h"
#include "Entity.h"
#include "renderer/Camera.h"
enum revolverState_t {
	REVOLVER_IDLE,
	REVOLVER_RELOADING,
	REVOLVER_SHOOTING,
};

enum revolverAnimation_t {
	REVOLVER_ANIM_RELOAD,
	REVOLVER_ANIM_SHOOT,
};

class Revolver : public Entity {
public:
	revolverState_t revolverState;
	Vec3 basePosition;
	Quat baseRotation;

	float muzzleFlashTime;
	float maxMuzzleFlashTime;
	float spread;
	float spreadDecayRate;
	int ammo;
};

class Player : public Entity {
public:
	Camera camera;
	Revolver revolver;
};




Player* CreatePlayer( Vec3 pos );
void UpdatePlayer( Entity* entity );
void RevolverUpdate( Player* player );
void PlayerOnHit( EntityHitInfo info );