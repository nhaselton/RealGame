#include "game/game.h"
#include "Physics/Physics.h"
#include <AL/al.h>
#include "Renderer/DebugRenderer.h"

Model* Chaingunner::model;
Model* Chaingunner::projectileModel;
SkeletonPose* Chaingunner::deadPose;

Chaingunner* CreateChaingunner(Vec3 pos) {
	Chaingunner* chaingunner = (Chaingunner*)NewEntity();
	CreateBoid(chaingunner);
	EntityGenerateRenderModel(chaingunner, Chaingunner::model, &globalArena);
	chaingunner->pos = pos;
	chaingunner->state = 0;
	chaingunner->health = 2;
	chaingunner->maxHealth = 2;
	chaingunner->currentAnimation = Chaingunner::model->animations[CG_ANIM_IDLE];

	chaingunner->bounds->bounds.center = Vec3(0, 2, 0);
	chaingunner->bounds->bounds.width = Vec3(1.1f, 2., .9f);
	chaingunner->bounds->offset = chaingunner->pos;
	chaingunner->renderModel->scale = Vec3(1);
	chaingunner->renderModel->rotation = Quat(1, 0, 0, 0);//glm::normalize(Quat(0, 0, 1, 0));

	chaingunner->shootCooldown = 3.0f;
	chaingunner->nextShootTime = gameTime + 1.0f;
	chaingunner->rotation = glm::normalize(Quat(1, 0, 0, 0));

	chaingunner->Update = ChaingunnerUpdate;
	chaingunner->RecievedAnimationEvent = ChaingunnerRecievedAnimEvent;
	chaingunner->OnHit = ChaingunnerOnHit;

	chaingunner->state = CG_IDLE;
	return chaingunner;
}


void ChaingunnerRecievedAnimEvent(Entity* entity , AnimationEvent* event) {
	if (event->type == ANIM_EVENT_SHOOT_PROJECTILE)
		ChaingunnerShootBullet(entity);
}

void ChaingunnerShootBullet( Entity* entity ) {
	Chaingunner* chaingunner = (Chaingunner*)entity;

	Vec3 start = entity->pos + Vec3(0, 2, 0);
	Vec3 velNormalized = glm::normalize(entityManager.player->pos - start);
	Projectile* p = NewProjectile( start, velNormalized * 15.0f, Vec3(.25f), true  );
	p->model.model = Chaingunner::projectileModel;
	p->model.scale = Vec3(.25f);

	p->model.rotation = glm::quatLookAt(velNormalized, glm::cross(velNormalized, Vec3(0, 1, 1)));
	p->model.rotation *= Quat(1, 1, 0, 1);
	p->model.rotation = glm::normalize(p->model.rotation);
	p->OnCollision = WizardBallCallback;
}

void ChaingunnerUpdate( Entity* entity ) {
	EntityAnimationUpdate(entity, dt);

	switch (entity->state) {
		case CG_IDLE: ChaingunnerIdle(entity); break;
		case CG_MOVING: ChaingunnerMoving(entity); break;
		case CG_DYING: ChaingunnerDying(entity); break;
		case CG_STAGGER: ChaingunnerStagger (entity); break;
		case CG_SHOOTING: ChaingunnerShooting(entity); break;
		default:LOG_ASSERT(LGS_GAME,"bad CG state\n");
	}

	//Do gravity after move because sometimes he bugs out and shoots upwards in the other order
	Vec3 gravity = Vec3(0, -10 * dt, 0);
	entity->pos = MoveAndSlide(entity->bounds, gravity, 0, true);
}

void ChaingunnerIdle(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*) entity;
	ChaingunnerMovingStart(entity);
}

void ChaingunnerShootingStart(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	chaingunner->state = CG_SHOOTING;
	chaingunner->startShootingTime = gameTime;
	EntityStartAnimation(chaingunner, CG_ANIM_SHOOT);
}

void ChaingunnerShooting(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;

	if (gameTime - chaingunner->startShootingTime > 1.5f) {
		ChaingunnerMovingStart(entity);
	}

	//TODO LOS check if player been gone too long
	EntityLookAtPlayer(entity);
}



void ChaingunnerMoving(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	float dist = glm::length(Vec2(entity->pos.x - entityManager.player->pos.x, entity->pos.z - entityManager.player->pos.z));
	if (gameTime - chaingunner->startMovingTime > 3.0f || dist <= .1f) {
		ChaingunnerShootingStart(entity);
	}

	EntityLookAtPlayer(entity);
	if ((entity->spawnFlags & SPAWN_FLAGS_PERCH) == 0) {
		entity->pos = MoveAndSlide(entity->bounds, entity->boidVelocity * dt, 3, true);
	}
}

void ChaingunnerMovingStart(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	chaingunner->state = CG_MOVING;

	chaingunner->startMovingTime = gameTime;

	EntityStartAnimation(chaingunner, CG_ANIM_RUN);

	//Find New Target position
	Vec3 player = entityManager.player->pos;
	//How far they want to stay
	float safeRange = 10.0f;

	Vec3 dir = chaingunner->pos - player;
	dir = glm::normalize(dir);
	dir.y = 0;

	//Do it relative to player not self
	chaingunner->target = player + dir;
}

void ChaingunnerDyingStart(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	EntityStartAnimation(entity, CG_ANIM_DEATH);
	chaingunner->state = CG_DYING;
}

void ChaingunnerDying(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	if (entity->currentAnimationPercent >= 1.0f) {
		RemoveEntity(entity);
	}
}

void ChaingunnerOnHit(EntityHitInfo info) {
	Chaingunner* chaingunner = (Chaingunner*)info.victim;

	if (chaingunner->state == CG_DYING)
		return;
	chaingunner->health -= info.damage;
	if (chaingunner->health > 0) {
		ChaingunnerStaggerStart(chaingunner);
	}
	else {
		ChaingunnerDyingStart(chaingunner);
	}
}

void ChaingunnerStaggerStart(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;
	EntityStartAnimation(entity, CG_ANIM_STAGGER);
	chaingunner->state = CG_STAGGER;
}

void ChaingunnerStagger(Entity* entity) {
	Chaingunner* chaingunner = (Chaingunner*)entity;

	if (chaingunner->currentAnimationPercent >= 1.0f) {
		ChaingunnerMovingStart(entity);
	}

}

void ChaingunnerLoadKVP(void* ent, char* key, char* value) {
	Chaingunner* chaingunner = (Chaingunner*)ent;

	if (!TryEntityField(chaingunner, key, value)) {
		LOG_WARNING(LGS_GAME, "chaingunner has no kvp %s : %s", key, value);
	}
}
