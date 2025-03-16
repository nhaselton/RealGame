#pragma once
#include "def.h"
#include "Entity.h"
#include "renderer/Camera.h"
#include "Resources/SoundManager.h"
#include "Game/Weapons.h"

class Player : public Entity {
public:
	Camera camera;
	struct AudioSource* audioSource;
	bool noclip;

	bool grounded;
	float yVel;

	//bitset for unlocked weapons
	u16 weapons;

	static Sound revolverFireSound;
	static Sound revolverReloadSound;
	struct Light* light;
	float lightStart;

	Weapon* currentWeapon;
	Revolver revolver;
	Shotgun shotgun;
	PlasmaGun plasmaGun;
	RocketLauncher rocketLauncher;
};

Player* CreatePlayer( Vec3 pos );
void UpdatePlayer( Entity* entity );
void PlayerOnHit( EntityHitInfo info );
void ConsoleToggleNoClip();
void PlayerLoadKVP( void* player, char* key, char* value );
void PlayerCheckPickups(Player* player);
void PlayerPickupItem(Pickup* pickup, class Entity* entity);