#pragma once
#include "def.h"
#include "Entity.h"

class Weapon : public Entity {
public:
	Vec3 baseOffset;
	Vec3 currentOffset;
	
	Quat baseRotation;
	Quat currentRotation;

	float muzzleFlashTime;
	float maxMuzzleFlashTime;
	float spread;
	float spreadDecayRate;
	int ammo;

	bool owned;
	bool fullAuto;
	bool altFullAuto;

	void (*Shoot) (class Player* player, class Weapon* entity);
	void (*AltShoot) (class Player* player, class Weapon* entity);
	void (*Equip) (class Player* player, class Weapon* entity);
	void (*Update) (class Player* player, class Weapon* entity);
};

enum revolverState_t {
	REVOLVER_IDLE,
	REVOLVER_RELOADING,
	REVOLVER_SHOOTING,
};

enum revolverAnimation_t {
	REVOLVER_ANIM_RELOAD,
	REVOLVER_ANIM_SHOOT,
};

class Revolver : public Weapon {
public:
	revolverState_t revolverState;
	float shootCooldown;
	float fastShootCooldown;
	float currentShootCooldown;
	float spread;
	float spreadDecayRate;
};

enum shotgunState_t {
	SHOTGUN_IDLE,
	SHOTGUN_RELOAD //Includes shoot
};

enum shotgunAnimation_t{
	SHOTGUN_ANIM_RELOAD = 0,
	SHOTGUN_ANIM_IDLE = 1,
};

class Shotgun : public Weapon {
public:
	int numPellets;
	float spreadRadians;
	int mag;
};

class PlasmaGun : public Weapon {
public:
	float currentCooldown;
	float shotCooldown;
};

enum rlState_t {
	RL_READY,
	RL_FIRE
};

enum rlAnimation_t {
	RL_ANIM_IDLE,
	RL_ANIM_FIRE
};

class RocketLauncher : public Weapon {
public:

};

void CreateRevolver(class Player* player);
void CreateShotgun(class Player* player);
void CreatePlasmaGun(class Player* player);
void CreateRocketLauncher(class Player* player);