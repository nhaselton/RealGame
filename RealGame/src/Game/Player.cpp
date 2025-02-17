#include "Player.h"
#include "Game.h"
#include "EntityManager.h"
#include "Physics\Physics.h"
#include "Resources/ModelManager.h"
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

	Model* revolverModel = ModelManagerAllocate( &modelManager, "res/models/revolver.glb" );

	player->revolver.renderModel = new RenderModel;
	player->revolver.renderModel->model = revolverModel;
	player->revolver.renderModel->rotation = Quat( 1.0, 0, 0, 0 );
	player->revolver.renderModel->scale = Vec3( 1 );
	player->revolver.renderModel->translation = Vec3( 0 );
	player->revolver.renderModel->pose = ( SkeletonPose* ) ScratchArenaAllocateZero( &globalArena, sizeof( SkeletonPose ) );
	player->revolver.renderModel->pose->globalPose = ( Mat4* ) ScratchArenaAllocateZero( &globalArena, revolverModel->skeleton->numNodes * sizeof( Mat4 ) );
	player->revolver.renderModel->pose->pose = ( JointPose* ) ScratchArenaAllocateZero( &globalArena, revolverModel->skeleton->numNodes * sizeof( JointPose ) );
	player->revolver.renderModel->pose->skeleton = revolverModel->skeleton;

	player->revolver.maxMuzzleFlashTime = .1f;
	player->revolver.animTimeScale = 1.0f;

	player->revolver.state = REVOLVER_RELOADING;
	player->revolver.currentAnimation = revolverModel->animations[0];

	player->revolver.basePosition = Vec3( .5f, -.4f, -1.1f );
	player->revolver.baseRotation = glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 92.0f ), Vec3( 0, 1, 0 ) );
	player->revolver.renderModel->scale = Vec3( .4f ) ;

	player->revolver.pos = player->revolver.basePosition;
	player->revolver.rotation = player->revolver.baseRotation;
	player->revolver.ammo = 6;
	player->revolver.spreadDecayRate = 6.0f;

	player->audioSource = NewAudioSource();

	player->OnHit = PlayerOnHit;
	
	Mat4 revolverScale = glm::scale( Mat4( 1.0 ), Vec3( .25f ) );
	Mat4 rot = glm::toMat4( glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 90.0f ), Vec3( 0, 1, 0 ) ) );

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
		wantDir.y = 0;
		EntityMove( player, wantDir );
	}
	else {
		entity->pos += wantDir;
		entity->bounds->offset = entity->pos;
	}

	player->camera.Position = player->pos + Vec3( 0, 1, 0 );
	RevolverUpdate(player);

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
}

void RevolverUpdate( Player* player ) {
	Revolver* revolver = &player->revolver;

	EntityAnimationUpdate( revolver, dt * 1.25f );
	
	revolver->spread -= revolver->spreadDecayRate * dt;
	if ( revolver->spread < 1.0f ) 
		revolver->spread = 1.0f;

	revolver->pos = glm::mix( revolver->pos, revolver->basePosition, .3f * dt );
	revolver->rotation = glm::slerp( revolver->rotation, revolver->baseRotation, 2.f * dt );

	//RELOAD STATE
	if ( revolver->state == REVOLVER_RELOADING ) {
		if ( revolver->currentAnimationPercent == 1.0f ) {
			revolver->state = REVOLVER_IDLE;
			revolver->ammo = 6;
			revolver->spread = 1.0f;
		}
		//Dont allow any actions
		return;
	}

	//SHOOT STATE
	if( KeyPressed( KEY_SPACE ) || ( window.cursorLocked && MousePressed( MOUSE_BUTTON_LEFT ) ) ) {
		if ( revolver->ammo == 0 ) {
			PlaySound( player->audioSource, &Player::revolverReloadSound );
			revolver->state = REVOLVER_RELOADING;
			EntityStartAnimation( revolver, REVOLVER_ANIM_RELOAD );
			return;
		}

		EntityStartAnimation( revolver, REVOLVER_ANIM_SHOOT );
		HitInfo info{};
		if ( PhysicsQueryRaycast( player->camera.Position, player->camera.Front * 100.0f, &info ) ) {
			if ( info.entity != 0 && info.entity->OnHit != 0 ) {
				EntityHitInfo attack{};
				attack.attacker = player;
				attack.victim = info.entity;
				attack.projectile = 0;
				attack.damage = 1;
				info.entity->OnHit( attack );
			}
		}

		AudioSource* fire = CreateTempAudioSource( player->camera.Position, &Player::revolverFireSound );
		if ( fire ) 
			alSourcef( fire->alSourceIndex, AL_GAIN, .25 );


		revolver->pos += Vec3( 0, .02, 0 );
		revolver->rotation = glm::rotate( revolver->rotation, -.3f, Vec3( 0, 0, -1 ) );
		revolver->spread += 2.0f;
		revolver->ammo--;
		revolver->muzzleFlashTime = gameTime + revolver->maxMuzzleFlashTime;
		return;
	}

	//DO RELOAD
	if ( KeyPressed( KEY_R ) && revolver->ammo != 6 ) {
		CreateTempAudioSource( player->camera.Position, &Player::revolverReloadSound );
		revolver->state = REVOLVER_RELOADING;
		revolver->spread = 1.0f;
		EntityStartAnimation( revolver, REVOLVER_ANIM_RELOAD );
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