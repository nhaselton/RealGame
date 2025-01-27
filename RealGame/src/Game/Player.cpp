#include "Player.h"
#include "EntityManager.h"
#include "Physics\Physics.h"
#include "Resources/ModelManager.h"

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

	Model* revolverModel = ModelManagerAllocate( &modelManager, "res/models/revolver2Anim.glb" );

	player->revolver.renderModel = new RenderModel;
	player->revolver.renderModel->model = revolverModel;
	player->revolver.renderModel->rotation = Quat( 1.0, 0, 0, 0 );
	player->revolver.renderModel->scale = Vec3( 1 );
	player->revolver.renderModel->translation = Vec3( 0 );
	player->revolver.renderModel->pose = ( SkeletonPose* ) ScratchArenaAllocateZero( &globalArena, sizeof( SkeletonPose ) );
	player->revolver.renderModel->pose->globalPose = ( Mat4* ) ScratchArenaAllocateZero( &globalArena, revolverModel->skeleton->numNodes * sizeof( Mat4 ) );
	player->revolver.renderModel->pose->pose = ( JointPose* ) ScratchArenaAllocateZero( &globalArena, revolverModel->skeleton->numNodes * sizeof( JointPose ) );
	player->revolver.renderModel->pose->skeleton = revolverModel->skeleton;

	player->revolver.state = REVOLVER_RELOADING;
	player->revolver.currentAnimation = revolverModel->animations[0];

	player->revolver.basePosition = Vec3( .5f, -.4f, -1.1f );
	player->revolver.baseRotation = glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 92.0f ), Vec3( 0, 1, 0 ) );
	player->revolver.renderModel->scale = Vec3( .4f ) ;

	player->revolver.pos = player->revolver.basePosition;
	player->revolver.rotation = player->revolver.baseRotation;
	player->revolver.ammo = 6;
	player->revolver.spreadDecayRate = 6.0f;

	player->OnHit = PlayerOnHit;
	
	Mat4 revolverScale = glm::scale( Mat4( 1.0 ), Vec3( .25f ) );
	Mat4 rot = glm::toMat4( glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 90.0f ), Vec3( 0, 1, 0 ) ) );

	return player;
}

void UpdatePlayer( Entity* entity ) {
	Player* player = ( Player* ) entity;
	
	float mx = 0, my = 0, speed = 15;
	if ( KeyDown( KEY_LEFT ) ) mx -= 3;
	if ( KeyDown( KEY_RIGHT ) ) mx += 3;
	if ( KeyDown( KEY_DOWN ) ) my -= 1.5;
	if ( KeyDown( KEY_UP ) ) my += 1.5;
	player->camera.ProcessMouseMovement( mx * speed * dt * 50, my * dt * speed * 50 );

	Vec3 wantDir( 0 );
	if ( KeyDown( KEY_W ) ) wantDir += player->camera.Front;
	if ( KeyDown( KEY_S ) ) wantDir -= player->camera.Front;
	if ( KeyDown( KEY_A ) ) wantDir += -player->camera.Right;
	if ( KeyDown( KEY_D ) ) wantDir += player->camera.Right;
	if ( wantDir != Vec3( 0 ) )
		wantDir = glm::normalize( wantDir );

	wantDir *= 20.0f * dt;
	EntityMove( player, wantDir );

	player->camera.Position = player->pos + Vec3( 0, 1, 0 );


	RevolverUpdate(player);
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
	if ( KeyPressed( KEY_SPACE ) ) {
		if ( revolver->ammo == 0 ) {
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
		revolver->pos += Vec3( 0, .02, 0 );
		revolver->rotation = glm::rotate( revolver->rotation, -.3f, Vec3( 0, 0, -1 ) );
		revolver->spread += 2.0f;
		revolver->ammo--;
		return;
	}

	//DO RELOAD
	if ( KeyPressed( KEY_R ) && revolver->ammo != 6 ) {
		revolver->state = REVOLVER_RELOADING;
		revolver->spread = 1.0f;
		EntityStartAnimation( revolver, REVOLVER_ANIM_RELOAD );
	}
}

void PlayerOnHit( EntityHitInfo info ) {
	printf( "ouch!" );

	info.victim->health--;
	if ( info.victim->health < 0 )
		exit( 0 );

	return;
}