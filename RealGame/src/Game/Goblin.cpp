#include "Game.h"
#include "Physics/Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer\DebugRenderer.h"
#include "Renderer\Renderer.h"
#include "Resources/SoundManager.h"
#include <AL/al.h>
Model* Goblin::model = 0;
Sound Goblin::staggerSound;

extern Sound explosion;
Goblin* CreateGoblin( Vec3 pos ) {
	//Todo Better Solution
	Goblin* goblin = ( Goblin* ) NewEntity();
	CreateBoid( goblin );
	EntityGenerateRenderModel( goblin, Goblin::model, &globalArena );
	goblin->pos = pos;
	goblin->currentAnimation = goblin->renderModel->model->animations[0];
	goblin->state = GOBLIN_CHASE;
	goblin->health = 2;
	goblin->maxHealth = 5;
	goblin->currentAnimation = Goblin::model->animations[0];
	goblin->renderModel->scale = Vec3( .45 );

	goblin->bounds->bounds.center = Vec3( 0, -.3, 0 );
	goblin->bounds->bounds.width = Vec3( 1.55f, 2, 1.55f );
	goblin->bounds->bounds.center += Vec3( -.1f, .6f, 0 );
	goblin->bounds->offset = goblin->pos;

	goblin->animTimeScale = 2.0f;

	goblin->Update = GoblinUpdate;
	goblin->OnHit = GoblinOnHit;

	goblin->audioSource = NewAudioSource();
	if( goblin->audioSource ) {
		alSourcef( goblin->audioSource->alSourceIndex, AL_REFERENCE_DISTANCE, 3.5f );
		alSourcef( goblin->audioSource->alSourceIndex, AL_ROLLOFF_FACTOR, 1.0f );
	}

	return goblin;
}

void GoblinUpdate( Entity* entity ) {
	Goblin* goblin = ( Goblin* ) entity;
	goblin->audioSource->pos = goblin->pos;

	switch ( (goblinStates_t) goblin->state ) {
		case GOBLIN_CHASE: GoblinChase( goblin ); break;
		case GOBLIN_STAGGER: GoblinStagger( goblin ); break;
	}
}

void GoblinChase( Goblin* goblin ) {
	EntityLookAtPlayer( goblin );

	goblin->target = entityManager.player->pos;
	goblin->target.y = goblin->pos.y;
	//Vec3 velocity = entityManager.player->pos - goblin->pos;
	//velocity.y = 0;

	Vec3 velocity = goblin->boidVelocity;
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
	Goblin* goblin = (Goblin*) info.victim;
	goblin->health-= info.damage;

	if ( goblin->health <= 0 ) {
		RemoveEntity( goblin );	
		RemoveBoid( goblin );
		EncounterNotifyOnDeath( goblin->encounter, goblin );


		//Explode
		for (int i = 0; i < entityManager.numEntities; i++) {
			if (entityManager.entities[i].state != ACTIVE_ACTIVE)
				continue;
			Entity* other = &entityManager.entities[i].entity;

			float dist = glm::length( other->pos - goblin->pos );
			if (dist > 10.0f )
				continue;

			if (other->OnHit != 0) {
				EntityHitInfo info{};
				info.attacker = goblin;
				info.victim = other;
				info.damage = 2.0f;
				other->OnHit ( info );
			}
		}
		CreateTempAudioSource( goblin->pos, &explosion );

		Vec3 velocities []{
			Vec3( -5,10,3 ),
			Vec3( 10,10,-7 ),
			Vec3( -3,10, 8 ),
		};

		Model* gibs = ModelManagerGetModel( "res/models/gib.glb" );

		//Smoke Emitter
#if 1
		ParticleEmitter2* emitter = NewParticleEmitter();
		emitter->pos = goblin->pos + Vec3( 0, 1, 0 );
		emitter->UV = Vec4( .03125, 0, .03125 + .03125, .03125 );
		emitter->maxEmitterLifeTime = 1.0f;
		emitter->maxParticles = 200;
		emitter->spawnRate = 10;
		emitter->scale = Vec2( 1.0f );
		emitter->acceleration = Vec3( 0, 5, 0 );
		emitter->radius = 6.0f;
		emitter->emitterSpawnType = EMITTER_INSTANT;
#endif	
		//Explosion Emitter
		ParticleEmitter2* emitter2 = NewParticleEmitter();
		emitter2->pos = goblin->pos + Vec3( 0, 1, 0 );
		emitter2->UV = Vec4( .03125 * 2, 0, .03125 * 3, .03125 );
		emitter2->maxEmitterLifeTime = 3.0f;
		emitter2->maxParticles = 60;
		emitter2->spawnRate = 10;
		emitter2->scale = Vec2( 2.5f );
		emitter2->acceleration = Vec3( 0, -1, 0 );
		emitter2->radius = 2.0f;
		emitter2->emitterSpawnType = EMITTER_INSTANT;

		for( int i = 0; i < 3; i++ ) {
			RigidBody* gib = NewRigidBody();
			if( gib ) {
				gib->pos = goblin->pos + Vec3( 0, 2, 0 );
				gib->velocity = velocities[i];
				gib->removeTime = gameTime + 10.0f;;
				float gibsize = ( ( float ) ( rand() % 2 + 1 ) ) / 2.0f;
				gib->radius = gibsize;
				gib->modelScale = gibsize;
				gib->model = gibs;
			}

			//Blood Emitter
#if 1
			ParticleEmitter2* emitter = NewParticleEmitter();
			emitter->pos = Vec3( 0, 0, 1 );
			emitter->UV = Vec4( 0, 0, .03125, .03125 );
			emitter->maxEmitterLifeTime = 3.0f;
			emitter->maxParticles = 400;
			emitter->spawnRate = 100;
			emitter->scale = Vec2( 0.6f );
			emitter->acceleration = Vec3( 0, -10, 0 );
			emitter->radius = 1.0f;
			emitter->emitterSpawnType = EMITTER_OVERTIME;

			if ( gib )
				gib->emitter = emitter;
#endif
		}
		return;
	}
	else {
		PlaySound( goblin->audioSource, &Goblin::staggerSound );
	}
	EntityStartAnimation( goblin, GOBLIN_ANIM_STAGGER );
	goblin->state = GOBLIN_STAGGER;
}

void GoblinLoad(Parser* parser) {
	Goblin* goblin = CreateGoblin( Vec3( 0 ) );

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		if( !TryEntityField( goblin, key, value ) ) {
			LOG_WARNING( LGS_GAME, "player has no kvp %s : %s", key, value );
		}

		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}
}