#pragma once
#include "def.h"
#include "Entity.h"
#include "renderer/Camera.h"
#include "Resources/SoundManager.h"
#include "Game/Weapons.h"

enum keyBits_t {
	PLAYER_KEY_RED =  1,
	PLAYER_KEY_BLUE = 2,
};

class Player : public Entity {
public:
	Camera camera;
	struct AudioSource* audioSource;
	bool noclip;

	bool grounded;
	float yVel;

	//bitset for unlocked weapons
	u16 weapons;
	u8 keys;

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
//Returns if item needs to disappear (Not everything can always be picked up)
bool PlayerPickupItem(Pickup* pickup, class Entity* entity);