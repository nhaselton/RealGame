#include "wizard.h"
#include"Game/Game.h"
#include "Physics/Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer/DebugRenderer.h"
#include "renderer/Renderer.h"

#include <AL/al.h>

Model* Wizard::model;
Model* Wizard::projectileModel;
SkeletonPose* Wizard::deadPose;
Sound Wizard::shootSound;
Sound Wizard::ballExplosionSound;
Sound Wizard::spotSound;
Sound Wizard::staggerSound;
Sound Wizard::deathSound;

void WizardStartShoot( Wizard* wizard ) {
	wizard->state = WIZARD_SHOOT;
	EntityStartAnimation( wizard, WIZARD_ANIM_SHOOT );
}

void WizardStartRepositioning( Wizard* wizard ) {
	wizard->state = WIZARD_REPOSITION;
	wizard->startMovingTime = gameTime;

	EntityStartAnimation( wizard, WIZARD_ANIM_RUN );

	//Find New Target position
	Vec3 player = entityManager.player->pos;
	//How far they want to stay
	float safeRange = 10.0f;

	Vec3 dir = wizard->pos - player;
	dir = glm::normalize( dir );
	dir.y = 0;

	//Do it relative to player not self
	wizard->target = player + dir;
}

void WizardStartMelee( Wizard* wizard ) {
	wizard->state = WIZARD_MELEE;
	EntityStartAnimation( wizard, WIZARD_ANIM_MELEE );
}

Wizard* CreateWizard( Vec3 pos ) {
	Wizard* wizard = ( Wizard* ) NewEntity();
	CreateBoid( wizard );
	EntityGenerateRenderModel( wizard, Wizard::model, &globalArena );
	wizard->pos = pos;
	wizard->state = 0;
	wizard->health = 2;
	wizard->maxHealth = 2;
	wizard->currentAnimation = Wizard::model->animations[0];

	wizard->bounds->bounds.center = Vec3( 0, 2.8, 0 );
	wizard->bounds->bounds.width = Vec3( 1.25f, 2.4, 1.25f );
	wizard->bounds->offset = wizard->pos;
	wizard->renderModel->scale = Vec3(2,2,2);

	wizard->shootCooldown = 3.0f;
	wizard->nextShootTime = gameTime + 1.0f;
	wizard->nextMelee = gameTime + 1.0f;

	wizard->Update = WizardUpdate;
	wizard->OnHit = WizardOnHit;
	wizard->RecievedAnimationEvent = WizardRecievedAnimationEvent;

	wizard->rotation = glm::normalize(Quat( 1, 0, 0, 0 ));

	wizard->audioSource = NewAudioSource();
	if( wizard->audioSource ) {
		alSourcef( wizard->audioSource->alSourceIndex, AL_REFERENCE_DISTANCE, 10.0f );
		alSourcef( wizard->audioSource->alSourceIndex, AL_ROLLOFF_FACTOR, 0.25f );
	}

	//wizard->state = WIZARD_IDLE;
	//EntityStartAnimation( wizard, WIZARD_ANIM_RUN );
	WizardStartRepositioning( wizard );
	return wizard;
}

void WizardUpdate( Entity* entity ) {
	Wizard* wizard = ( Wizard* ) entity;
	wizard->audioSource->pos = wizard->pos;

	Vec3 gravity = Vec3( 0, -10 * dt, 0 );
	wizard->pos = MoveAndSlide( wizard->bounds, gravity, 0, true );

	switch( wizard->state ) {
		case WIZARD_IDLE:
		WizardIdle( wizard );
		break;
		case WIZARD_MELEE:
		WizardMelee( wizard );
		break;
		case WIZARD_REPOSITION:
		WizardReposition( wizard );
		break;
		case WIZARD_SHOOT:
		WizardShoot( wizard );
		break;
		case WIZARD_STAGGER:
		WizardStagger( wizard );
		break;
		case WIZARD_DEATH:
		WizardDie( wizard );
		break;
	}
}

void WizardIdle( Wizard* wizard ) {
	Vec3 forward = EntityForward( wizard );
	forward.y = 0;
	Vec3 playerPos = entityManager.player->pos;
	playerPos.y = 0;

	//Check if player In Front
	float cosAngle = glm::dot( forward, playerPos - forward );
	if( cosAngle < 0 ) 
		return;

	HitInfo info{};
	if( PhysicsQueryRaycast( wizard->pos, entityManager.player->pos - wizard->pos, &info ) ) {
		//Look into global timer so this doesnt happen too much maybe?
		//PlaySound( wizard->audioSource, &Wizard::spotSound );
		if( info.entity == entityManager.player )
			WizardStartRepositioning( wizard );
	}

}

void WizardShoot( Wizard* wizard ) {
	EntityLookAtPlayer( wizard );

	if( wizard->currentAnimationPercent >= 1.0f ) {
		wizard->nextShootTime = gameTime + wizard->shootCooldown;
		WizardStartRepositioning( wizard );
		return;
	}

}

void WizardReposition( Wizard* wizard ) {
	EntityLookAtPlayer( wizard );

	if( wizard->nextMelee <= gameTime 
		&& glm::length( entityManager.player->pos - wizard->pos ) < 7.0f ) {
		WizardStartMelee(wizard);
		return;
	}
	
	//Try and shoot
	if( wizard->nextShootTime < gameTime ) {
		WizardStartShoot(wizard);
		return;
	}

	//If traveling for too long reposition
	if( wizard->startMovingTime > gameTime + 3.0f ) {
		WizardStartRepositioning( wizard );
		return;
	}

	//Just reposition
	Vec3 velocity = wizard->boidVelocity;
	if( glm::length2( velocity ) != 0 ) {
		velocity = glm::normalize( velocity ) * 10.0f * dt;
		//EntityMove( wizard, velocity );

		Vec3 wantMove =  MoveAndSlide( wizard->bounds, velocity, 3, false );
		SweepInfo info{};
		if ( PhysicsRaycastStaticFast(wantMove,Vec3(0,-3,0)) ){
			//if( PhysicsQuerySweepStatic( wantMove, Vec3( 0, -5, 0 ), wizard->bounds->bounds.width * .75f, &info ) ) {
			wizard->pos = wantMove;
			wizard->bounds->offset = wizard->pos;
		}
	}
}

void WizardMelee( Wizard* wizard ) {
	if( wizard->currentAnimationPercent >= 1.0f ) {
		wizard->nextMelee = gameTime + wizard->shootCooldown;
		wizard->nextShootTime = gameTime + wizard->shootCooldown / 2.0f; //Dont immediately shoot after a melee
		WizardStartRepositioning( wizard );
		return;
	}
}

void WizardStartDeath(Wizard* wizard) {
	AudioSource* death = CreateTempAudioSource( wizard->pos, &Wizard::deathSound );
	death->pos = wizard->pos;
	alSourcef( death->alSourceIndex, AL_GAIN, 2.0f );



	EntityStartAnimation( wizard, WIZARD_ANIM_DEATH );
	wizard->state = WIZARD_DEATH;
	EncounterNotifyOnDeath( wizard->encounter, wizard );
	RemoveAudioSource( wizard->audioSource );
	RemoveBoid( wizard );
}

void WizardDie( Wizard* wizard ) {
	if( wizard->currentAnimationPercent >= 1.0f ) {
		RigidBody* deadBody = NewRigidBody();
		deadBody->pos = wizard->pos + wizard->bounds->bounds.center / 2.0f;
		deadBody->removeTime = gameTime + 10.0f;;
		deadBody->radius = wizard->bounds->bounds.width.x;
		deadBody->modelScale = wizard->renderModel->scale.x;
		deadBody->model = wizard->model;
		deadBody->pose = Wizard::deadPose;
		RemoveEntity( wizard );
	}
}

void WizardOnHit( EntityHitInfo info ) {
	Wizard* wizard = ( Wizard* ) info.victim;

	if( wizard->state == WIZARD_DEATH )
		return;
	wizard->health -= info.damage;
	if( wizard->health > 0 ) {
		WizardStartStagger( wizard );
	}
	else {
		WizardStartDeath( wizard );
	}
}

void WizardStartStagger( Wizard* wizard ) {
	EntityStartAnimation( wizard, WIZARD_ANIM_STAGGER );
	PlaySound( wizard->audioSource, &Wizard::staggerSound );
	wizard->state = WIZARD_STAGGER;
}

void WizardStagger( Wizard* wizard ) {
	if( wizard->currentAnimationPercent >= 1.0f ) {
		WizardStartRepositioning( wizard );
		return;
	}
}

void WizardBallCallback( class Projectile* projectile, class Entity* entity ) {
	if( entity && entity != projectile->owner && entity->OnHit ) {
		EntityHitInfo info{};
		info.attacker = projectile->owner;
		info.victim = entity;
		info.projectile = projectile;
		info.damage = 1;
		entity->OnHit( info );
	}

	CreateTempAudioSource( projectile->collider.offset, &Wizard::ballExplosionSound );
	RemoveProjectile( projectile );
	ParticleEmitter2* emitter = NewParticleEmitter();
	if( !emitter )
		return;
	emitter->numParticles = 300;
	emitter->maxParticles = 300;
	emitter->scale = Vec3( .2f );
	emitter->acceleration = Vec3( 0, -20, 0 );
	emitter->emitterSpawnType = EMITTER_INSTANT;
	emitter->maxEmitterLifeTime = 1.5f;
	emitter->particleLifeTime = 3.0f;
	emitter->radius = 2.0f;
	emitter->UV = Vec4( .09375, 0, 0.125, .03125 );
	emitter->pos = projectile->collider.offset;	
}

void WizardRecievedAnimationEvent( Entity* ent, AnimationEvent* event ) {
	Wizard* wizard = ( Wizard* ) ent;
	switch( event->type ) {
		case ANIM_EVENT_SHOOT_PROJECTILE:
		{

		Vec3 orbPos = wizard->pos + Vec3( 0, 3, 0 );
		Vec3 velocity = glm::normalize( ( entityManager.player->pos - orbPos ) ) * 40.0f;
		Projectile* orb = NewProjectile( orbPos, velocity, Vec3( .5f ), true );
		if( orb ) {
			orb->collider.owner = wizard;
			orb->model.model = Wizard::projectileModel;
			orb->model.scale = Vec3( .5f );
			orb->model.translation = Vec3( 0 );
			orb->OnCollision = WizardBallCallback;
			//PlaySound( wizard->audioSource, &Wizard::shootSound );
		}
		}break;
		case ANIM_EVENT_MELEE_ATTACK:
		{
			ParticleEmitter2* emitter = NewParticleEmitter();
			emitter->pos = wizard->pos + Vec3( 0, 0, 0 );
			//emitter->UV 
			emitter->numParticles = 300;
			emitter->maxParticles = 300;
			emitter->scale = Vec3( .2f );
			emitter->acceleration = Vec3( 0, -20, 0 );
			emitter->emitterSpawnType = EMITTER_INSTANT;
			emitter->maxEmitterLifeTime = 1.5f;
			emitter->particleLifeTime = 3.0f;
			emitter->radius = 2.0f;
			emitter->UV = Vec4( .09375, 0, 0.125, .03125 );
		}break;
		
	}
}

void WizardLoad( Parser* parser ) {
	Wizard* wizard = CreateWizard( Vec3( 0 ) );

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		if( !TryEntityField( wizard, key, value ) ) {
			LOG_WARNING( LGS_GAME, "wizard has no kvp %s : %s", key, value );
		}

		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}
}

