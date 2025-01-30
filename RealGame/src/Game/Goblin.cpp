#include "Game.h"
#include "Physics/Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer\DebugRenderer.h"

Model* Goblin::model = 0;

Goblin* CreateGoblin( Vec3 pos ) {
	//Todo Better Solution
	Goblin* goblin = ( Goblin* ) NewEntity();
	EntityGenerateRenderModel( goblin, Goblin::model, &globalArena );
	goblin->pos = pos;
	goblin->currentAnimation = goblin->renderModel->model->animations[0];
	goblin->state = GOBLIN_CHASE;
	goblin->health = 1;
	goblin->maxHealth = 5;
	goblin->currentAnimation = Goblin::model->animations[0];
	goblin->renderModel->scale = Vec3( .45 );

	goblin->pos = Vec3( 0, -.3, 0 );
	goblin->bounds->bounds.width = Vec3( 1.55f, 2, 1.55f );
	goblin->bounds->bounds.center += Vec3( -.1f, .6f, 0 );
	goblin->bounds->offset = goblin->pos;

	goblin->animTimeScale = 2.0f;

	goblin->Update = GoblinUpdate;
	goblin->OnHit = GoblinOnHit;
	return goblin;
}

void GoblinUpdate( Entity* entity ) {
	Goblin* goblin = ( Goblin* ) entity;

	switch ( (goblinStates_t) goblin->state ) {
		case GOBLIN_CHASE: GoblinChase( goblin ); break;
		case GOBLIN_STAGGER: GoblinStagger( goblin ); break;
	}
}

void GoblinChase( Goblin* goblin ) {
	EntityLookAtPlayer( goblin );

	Vec3 velocity = entityManager.player->pos - goblin->pos;
	velocity.y = 0;

	if ( glm::length2( velocity ) != 0 ) {
		velocity = glm::normalize( velocity ) * 10.0f * dt;
		EntityMove( goblin, velocity );
	}
}

void GoblinStagger( Goblin* goblin ) {
	if ( goblin->currentAnimationPercent == 1.0f ) {
		goblin->state = GOBLIN_CHASE;
		EntityStartAnimation( goblin, GOBLIN_ANIM_RUN );
	}
}


void GoblinOnHit( EntityHitInfo info ) {
	Entity* goblin = info.victim;

	goblin->health--;
	if ( goblin->health <= 0 ) {
		RemoveEntity( goblin );	

		Vec3 velocities []{
			Vec3( -5,10,3 ),
			Vec3( 10,10,-7 ),
			Vec3( -3,10, 8 ),
		};

		Model* gibs = ModelManagerGetModel( "res/models/gib.glb" );

		for ( int i = 0; i < 3; i++ ) {
			RigidBody* gib = NewRigidBody();
			gib->pos = goblin->pos + Vec3(0,2,0);
			gib->velocity = velocities[i];

			float gibsize = ( ( float ) ( rand() % 2 + 1 ) ) / 2.0f;
			gib->radius = gibsize;
			gib->modelScale = gibsize;
			gib->model = gibs;
		}
		return;
	}

	EntityStartAnimation( goblin, GOBLIN_ANIM_STAGGER );
	goblin->state = GOBLIN_STAGGER;
}