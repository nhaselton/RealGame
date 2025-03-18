#include "Boar.h"
#include "Physics/Physics.h"
#include <AL/al.h>
#include "Renderer/DebugRenderer.h"
#include "Game/Game.h"

Model* Boar::model;
SkeletonPose* Boar::deadPose;

Boar* CreateBoar(Vec3 pos) {
	Boar* boar = (Boar*)NewEntity();
	CreateBoid(boar);
	EntityGenerateRenderModel(boar, Boar::model, &globalArena);
	boar->pos = pos;
	boar->state = 0;
	boar->health = 48;
	boar->maxHealth = 48;
	boar->currentAnimation = Boar::model->animations[BOAR_ANIM_IDLE];

	boar->bounds->bounds.center = Vec3(0, 2.75f, 0.5);
	boar->bounds->bounds.width = Vec3(1.7f, 2.75f, 4.00f);
	boar->bounds->offset = boar->pos;
	boar->renderModel->scale = Vec3(1.5f, 1, 1);
	boar->renderModel->rotation = Quat(1, 0, 0, 0);//glm::normalize(Quat(0, 0, 1, 0));

	boar->rotation = glm::normalize(Quat(1, 0, 0, 0));

	boar->Update = BoarUpdate;
	boar->RecievedAnimationEvent = BoarRecievedAnimationEvent;
	boar->OnHit = BoarHit;

	boar->state = BOAR_IDLE;
	return boar;
}

void BoarUpdate(Entity* entity) {
	Boar* boar = (Boar*)entity;

	EntityAnimationUpdate(entity, dt);
	DebugDrawCharacterCollider(entity->bounds);

	switch (boar->state) {
	case BOAR_IDLE: EntityLookAtPlayer(entity);  BoarStartCharge(entity); break;
	case BOAR_CHARGE: BoarCharge(entity); break;
	case BOAR_HEADBUTT: break;
	case BOAR_STAGGER: BoarStagger(entity); break;
		
	}


	Vec3 gravity = Vec3(0, -10 * dt, 0);
	entity->pos = MoveAndSlide(entity->bounds, gravity, 0, true);
}

void BoarStartCharge(Entity* entity ) {
	Boar* boar = (Boar*)entity;
	EntityLookAtPlayer(entity);
	EntityStartAnimation(entity, BOAR_ANIM_CHARGE);
	boar->state = BOAR_CHARGE;
	boar->startVel = entityManager.player->pos - entity->pos;
	boar->startVel.y = 0;
}

void BoarCharge(Entity* entity) {
	Boar* boar = (Boar*)entity;
	// Slowly Turns Towards Player
	Vec3 dir = entityManager.player->pos - entity->pos;
	dir.y = 0;  // Keep it horizontal, preventing unnecessary pitch rotation.
	dir = glm::normalize(dir);  // Normalize the direction vector.

	// Compute the target rotation quaternion to face the player.
	Quat finalRot = glm::quatLookAt(-dir, Vec3(0, 1, 0));

	// Get the angle between the current rotation and the target rotation
	float angle = glm::angle(entity->rotation * glm::inverse(finalRot));
	float rotationSpeed = 1.0f; 
	// Apply the rotation incrementally
	if ( angle > 0)
		entity->rotation = glm::slerp(entity->rotation, finalRot, (1.0f * dt) / angle);

	Vec3 forward = EntityForward(entity);
	entity->pos = MoveAndSlide(entity->bounds, forward * 30.0f * dt, 3, true);

	//
	Vec3 fwCheck(forward.x, 0, forward.z);
	if (glm::dot(fwCheck, boar->startVel) < 0.0f) {
		boar->state = BOAR_STAGGER;
		EntityStartAnimation(boar, BOAR_ANIM_IDLE);
		return;
	}


	//check if crashed into wall
	SweepInfo info{};
	if (PhysicsQuerySweepStatic(entity->pos + entity->bounds->bounds.center, forward / 10.0f, entity->bounds->bounds.width, &info)) {
		boar->state = BOAR_STAGGER;
		EntityStartAnimation(boar, BOAR_ANIM_IDLE);
		return;
	}

}

void BoarHit(EntityHitInfo info) {
	Boar* boar = (Boar*)info.victim;
	boar->health -= info.damage;

	if (boar->health <= 0) {
		RemoveEntity(boar);
		RemoveBoid( boar );
	}
	else {
		boar->state = BOAR_STAGGER;
		EntityStartAnimation(boar, BOAR_ANIM_IDLE);
	}
}

void BoarStagger(Entity* entity) {
	if (entity->currentAnimationPercent == 1.0f) {
		entity->state = BOAR_IDLE;
	}
}

void BoarRecievedAnimationEvent(Entity* entity, AnimationEvent* event) {

}

void BoarLoadKVP(void* ent, char* key, char* value) {
	Boar* boar = (Boar*)ent;
	if (!TryEntityField(boar, key, value)) {
		LOG_WARNING(LGS_GAME, "Boar has no kvp %s : %s", key, value);
	}
}
