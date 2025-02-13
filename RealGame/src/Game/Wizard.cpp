#include "wizard.h"
#include"Game/Game.h"
#include "EntityManager.h"
#include "Physics/Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer/DebugRenderer.h"
#include "renderer/Renderer.h"
Model* Wizard::model;
SkeletonPose* Wizard::deadPose;

void WizardStartShoot( Wizard* wizard ) {
	wizard->state = WIZARD_SHOOT;
	EntityStartAnimation( wizard, WIZARD_ANIM_SHOOT );
}

void WizardStartRepositioning( Wizard* wizard ) {
	wizard->hasShot = false;
	wizard->hasMelee = false;
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
	
	WizardStartRepositioning( wizard );
	return wizard;
}

void WizardUpdate( Entity* entity ) {
	Wizard* wizard = ( Wizard* ) entity;

	switch( wizard->state ) {
		case WIZARD_IDLE:
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



void WizardShoot( Wizard* wizard ) {
	EntityLookAtPlayer( wizard );

	if( wizard->currentAnimationPercent >= 0.5f && !wizard->hasShot ) {
		Projectile* orb = NewProjectile();
		if( orb ) {
			orb->collider.offset = wizard->pos + Vec3( 0, .25, 0 );
			orb->collider.bounds.width = Vec3( 2.0f );
			orb->collider.bounds.center = Vec3( 0 );
			orb->collider.owner = wizard;
			orb->velocity = glm::normalize( ( entityManager.player->pos - orb->collider.offset ) ) * 20.0f;
			orb->model.model = Wizard::model;
			orb->model.scale = Vec3( .25f );
			orb->model.translation = Vec3( 0 );
			orb->OnCollision = 0;
			wizard->hasShot = true;
			return;
		}
	}

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
		EntityMove( wizard, velocity );
	}
}

void WizardMelee( Wizard* wizard ) {
	if( wizard->currentAnimationPercent >= 0.9f && !wizard->hasMelee ) {
		wizard->hasMelee = true;
		ParticleEmitter2* emitter = NewParticleEmitter();
		emitter->pos = wizard->pos + Vec3(0,0,0);
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

		return;
	}


	if( wizard->currentAnimationPercent >= 1.0f ) {
		wizard->nextMelee = gameTime + wizard->shootCooldown;
		wizard->nextShootTime = gameTime + wizard->shootCooldown / 2.0f; //Dont immediately shoot after a melee
		WizardStartRepositioning( wizard );
		return;
	}
}

void WizardStartDeath(Wizard* wizard) {
	EntityStartAnimation( wizard, WIZARD_ANIM_DEATH );
	wizard->state = WIZARD_DEATH;
	EncounterNotifyOnDeath( wizard->encounter, wizard );

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
	wizard->state = WIZARD_STAGGER;
}

void WizardStagger( Wizard* wizard ) {
	if( wizard->currentAnimationPercent >= 1.0f ) {
		WizardStartRepositioning( wizard );
		return;
	}
}
