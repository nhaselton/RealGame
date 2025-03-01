#pragma once
#include "def.h"
#include "game/entity.h"
#include "Resources/SoundManager.h"

enum wizardStates_t {
	WIZARD_IDLE = 0,
	WIZARD_REPOSITION,
	WIZARD_SHOOT,
	WIZARD_MELEE,
	WIZARD_STAGGER,
	WIZARD_DEATH,
	WIZARD_LAST
};

enum wizardAnimations_t {
	//WIZARD_ANIM_IDLE,
	WIZARD_ANIM_DEATH,
	WIZARD_ANIM_MELEE,
	WIZARD_ANIM_RUN,
	WIZARD_ANIM_SHOOT,
	WIZARD_ANIM_STAGGER,
};

ENT_CLASS Wizard : public Entity {
public:
	ENT_PARENT( "Entity" )
	EVAR float nextShootTime; 
	EVAR float shootCooldown;
	EVAR float nextMelee;
	EVAR float startMovingTime;
	class AudioSource* audioSource;


	static EPATH Model* model;
	static Model* projectileModel;
	static EPATH Projectile* projectile;

	static class Sound shootSound;
	static class Sound ballExplosionSound;
	static class Sound spotSound;
	static class Sound staggerSound;
	static class Sound deathSound;

	static SkeletonPose* deadPose;

	static Wizard prototype;
};

void WizardUpdate( Entity* entity );
void WizardOnHit( EntityHitInfo info );

Wizard* CreateWizard( Vec3 pos );

void WizardIdle( Wizard* wizard );
void WizardReposition( Wizard* wizard );
void WizardShoot( Wizard* wizard );
void WizardStagger( Wizard* wizard );
void WizardStartStagger( Wizard* wizard );
void WizardMelee( Wizard* wizard );
void WizardDie( Wizard* wizard );
void WizardRecievedAnimationEvent( Entity* wizard, struct AnimationEvent* event );

void WizardBallCallback( class Projectile* projectile, class Entity* entity );
void WizardLoad( Parser* parser );
void WizardLoadDefFile( const char* path );