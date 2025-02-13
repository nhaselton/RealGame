#pragma once
#include "def.h"
#include "game/entity.h"

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

class Wizard : public Entity {
public:
	float nextShootTime;
	float shootCooldown;
	float nextMelee;
	float startMovingTime;
	bool hasShot;
	bool hasMelee;

	static Model* model;
	static Model* projectileModel;
	static SkeletonPose* deadPose;
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

void WizardBallCallback( class Projectile* projectile, class Entity* entity );