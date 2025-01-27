#include "Player.h"
#include "EntityManager.h"
#include "Physics\Physics.h"

Player* CreatePlayer( Vec3 pos ) {
	Player* player = ( Player* ) NewEntity();
	entityManager.player = player;
	player->pos = pos;
	player->bounds->offset = player->pos;
	player->bounds->bounds.center = Vec3( 0 );
	player->bounds->bounds.width = Vec3( 1, 2, 1 );
	player->renderModel = 0;

	player->camera = Camera();

	player->Update = UpdatePlayer;
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

	if ( KeyDown( KEY_SPACE ) ) {
		PROFILE( "RAY" );
  		HitInfo info{};
		if ( PhysicsQueryRaycast( player->camera.Position, player->camera.Front * 100.0f, &info ) ) {
			if ( info.entity != 0 && info.entity->OnHit != 0 ) {
				Entity* e = info.entity;
				e->OnHit( e, 0 );
			}
		}
	}
}
