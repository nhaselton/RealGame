#include "Player.h"
#include "Game.h"
#include "EntityManager.h"
#include "Physics\Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer/Renderer.h"
#include "Resources/SoundManager.h"
#include "AL/al.h"

Sound Player::revolverFireSound;
Sound Player::revolverReloadSound;

Player* CreatePlayer( Vec3 pos ) {
	Player* player = ( Player* ) NewEntity();
	entityManager.player = player;
	player->pos = pos;
	player->bounds->offset = player->pos;
	player->bounds->bounds.center = Vec3( 0 );
	player->bounds->bounds.width = Vec3( 1, 2, 1 );
	player->renderModel = 0;

	player->health = 4;
	player->maxHealth = 4;

	player->camera = Camera();
	player->Update = UpdatePlayer;

	/* Create Revolver */
	memset( &player->revolver, 0, sizeof( player->revolver ) );

	player->audioSource = NewAudioSource();

	player->OnHit = PlayerOnHit;

	Mat4 revolverScale = glm::scale( Mat4( 1.0 ), Vec3( .25f ) );
	Mat4 rot = glm::toMat4( glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 90.0f ), Vec3( 0, 1, 0 ) ) );

	CreateRevolver(player);
	CreateShotgun(player);
	player->currentWeapon = &player->shotgun;

	return player;
}

void UpdatePlayer( Entity* entity ) {
	Player* player = ( Player* ) entity;
	player->audioSource->pos = player->camera.Position;

	if( !window.cursorLocked ) {
		float mx = 0, my = 0, speed = 15;
		if ( KeyDown( KEY_LEFT ) ) mx -= 3;
		if ( KeyDown( KEY_RIGHT ) ) mx += 3;
		if ( KeyDown( KEY_DOWN ) ) my -= 1.5;
		if ( KeyDown( KEY_UP ) ) my += 1.5;
		player->camera.ProcessMouseMovement( mx * speed * dt * 50, my * dt * speed * 50 );
	}
	else {
		player->camera.ProcessMouseMovement( xOffset, yOffset, true );
	}

	Vec3 wantDir( 0 );
	if ( KeyDown( KEY_W ) ) wantDir += player->camera.Front;
	if ( KeyDown( KEY_S ) ) wantDir -= player->camera.Front;
	if ( KeyDown( KEY_A ) ) wantDir += -player->camera.Right;
	if ( KeyDown( KEY_D ) ) wantDir += player->camera.Right;
	if ( wantDir != Vec3( 0 ) )
		wantDir = glm::normalize( wantDir );

	wantDir *= 20.0f * dt;

	//Dont move when console is open
	if( console.IsOpen() )
		wantDir = Vec3( 0 );

	if( !player->noclip ) {
		//Move
		wantDir.y = 0;
		player->pos = MoveAndSlide(player->bounds, wantDir, 3, true);

		//Check Grounded
		bool grounded = GroundCheck(player->pos, .15f, player->bounds->bounds.width);
		if (grounded) {
			player->grounded = true;
			player->yVel = 0.0f;

			if (KeyPressed(KEY_SPACE)) {
				player->yVel = 20.0f;
				//player->pos += Vec3(0, 0.1f, 0);
				player->bounds->offset = player->pos;
				player->pos = MoveAndSlide(player->bounds, Vec3(0, player->yVel * dt, 0), 0, true);
				grounded = false;
			}

		}
		if ( !grounded )
			player->yVel -= 20.0f * dt;

		player->pos = MoveAndSlide(player->bounds, Vec3(0, player->yVel * dt, 0), 0, true);

		if (!grounded)
			player->yVel -= 20.0f * dt;

	}
	else {
		entity->pos += wantDir;
		entity->bounds->offset = entity->pos;
	}

	//Check Triggers
	Vec3 center = player->bounds->bounds.center + player->pos;
	BoundsMinMax playerBounds{
	playerBounds.min =  center - player->bounds->bounds.width,
	playerBounds.max = center + player->bounds->bounds.width,
	};
	
	for( int i = 0; i < entityManager.numTriggers; i++ ) {
		Trigger* trigger = &entityManager.triggers[i];
		if( FastAABB( playerBounds, trigger->bounds ) ) {
			TriggerTrigger(trigger);
		}
	}

	//Update Weapon
	player->camera.Position = player->pos + Vec3(0, 1, 0);
	player->currentWeapon->Update(player, player->currentWeapon);

	if (player->currentWeapon->fullAuto) {
		if (KeyDown(KEY_LEFT_CONTROL) || (window.cursorLocked && MouseDown(MOUSE_BUTTON_LEFT)))
			player->currentWeapon->Shoot(player, player->currentWeapon);
	}
	else {
		if (KeyPressed(KEY_LEFT_CONTROL) || (window.cursorLocked && MousePressed(MOUSE_BUTTON_LEFT)))
			player->currentWeapon->Shoot(player, player->currentWeapon);
	}

	if (player->currentWeapon->altFullAuto && window.cursorLocked && MouseDown(MOUSE_BUTTON_RIGHT))
		player->currentWeapon->AltShoot(player, player->currentWeapon);

	else if (!player->currentWeapon->altFullAuto && window.cursorLocked && MousePressed(MOUSE_BUTTON_RIGHT))
		player->currentWeapon->AltShoot(player, player->currentWeapon);


	if( player->light && ( player->lightStart + .2f < gameTime ) ) {
		RemoveLight( player->light );
		player->light = 0;
	}
}

void PlayerOnHit( EntityHitInfo info ) {
	printf( "ouch!" );

	info.victim->health--;
	if (info.victim->health <= 0) {
		LOG_INFO ( LGS_GAME, "PLAYER DEAD!" );
		//exit( 0 );
	}

	return;
}

void ConsoleToggleNoClip() {
	Player* p = entityManager.player;
	if( !p ) 
		return;

	p->noclip = !p->noclip;
}

void PlayerLoad(Parser* parser) {
	Player* player = CreatePlayer( Vec3( 0 ) );
	if( entityManager.player != 0 ) {
		LOG_ERROR( LGS_GAME, "Trying to create a new player when one already exists\n" );
	}
	entityManager.player = player;

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		if( !TryEntityField( player, key, value ) ) {
			LOG_WARNING( LGS_GAME, "player has no kvp %s : %s", key, value );
		}

		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}
}