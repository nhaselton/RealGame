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

	static Sound revolverFireSound;
	static Sound revolverReloadSound;
	struct Light* light;
	float lightStart;

	Weapon* currentWeapon;
	Revolver revolver;
	Shotgun shotgun;
};

Player* CreatePlayer( Vec3 pos );
void UpdatePlayer( Entity* entity );
void RevolverUpdate( Player* player );
void PlayerOnHit( EntityHitInfo info );
void ConsoleToggleNoClip();
void PlayerLoad( Parser* parser );