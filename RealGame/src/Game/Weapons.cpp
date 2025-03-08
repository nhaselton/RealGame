#include "Game.h"
#include "Resources/ModelManager.h"
#include "Physics/Physics.h"
#include <AL/al.h>
#include <Renderer/DebugRenderer.h> 
void CreatePlayerMuzzleLight(Weapon* weapon, Player* player) {
	if (!player->light)
		player->light = NewLight();

	//If no free lights just return
	if (!player->light)
		return;

	player->light->pos = player->camera.Position + player->camera.Front;
	player->light->color = Vec3(1, .6, 0);
	player->light->intensity = 4.0f;
	player->light->type = LIGHT_POINT;
	LightSetAttenuation(player->light, 50);
	weapon->muzzleFlashTime = gameTime + weapon->maxMuzzleFlashTime;
	player->lightStart = gameTime;
}

void RevolverEquip(Player* player, Weapon* weapon) {
	Revolver* revolver = (Revolver*)weapon;

}

void RevolverUpdate(Player* player, Weapon* weapon) {
	Revolver* revolver = (Revolver*)weapon;
	EntityAnimationUpdate(revolver, dt * 1.25f);

	revolver->spread -= revolver->spreadDecayRate * dt;
	if (revolver->spread < 1.0f)
		revolver->spread = 1.0f;

	revolver->pos = glm::mix(revolver->pos, revolver->baseOffset, .3f * dt);
	revolver->rotation = glm::slerp(revolver->rotation, revolver->baseRotation, 2.f * dt);

	if (revolver->state == REVOLVER_RELOADING) {
		if (revolver->currentAnimationPercent == 1.0f) {
			revolver->state = REVOLVER_IDLE;
			revolver->ammo = 6;
			revolver->spread = 1.0f;
		}
	}

	if (KeyPressed(KEY_R) && revolver->ammo != 6) {
		CreateTempAudioSource(player->camera.Position, &Player::revolverReloadSound);
		revolver->state = REVOLVER_RELOADING;
		revolver->spread = 1.0f;
		EntityStartAnimation(revolver, REVOLVER_ANIM_RELOAD);
	}

}

void RevolverShoot(Player* player, Weapon* weapon) {
	Revolver* revolver = (Revolver*)weapon;
	if (revolver->state == REVOLVER_RELOADING)
		return;

	//Try to shoot with no ammo in clip
	if (revolver->ammo == 0) {
		PlaySound(player->audioSource, &Player::revolverReloadSound);
		revolver->state = REVOLVER_RELOADING;
		EntityStartAnimation(revolver, REVOLVER_ANIM_RELOAD);
		return;
	}

	EntityStartAnimation(revolver, REVOLVER_ANIM_SHOOT);
	HitInfo info{};
	if (PhysicsQueryRaycast(player->camera.Position, player->camera.Front * 100.0f, &info)) {
		if (info.entity != 0 && info.entity->OnHit != 0) {
			EntityHitInfo attack{};
			attack.attacker = player;
			attack.victim = info.entity;
			attack.projectile = 0;
			attack.damage = 1;
			info.entity->OnHit(attack);
		}
	}

	AudioSource* fire = CreateTempAudioSource(player->camera.Position, &Player::revolverFireSound);
	if (fire)
		alSourcef(fire->alSourceIndex, AL_GAIN, .25);
	CreatePlayerMuzzleLight(weapon, player);

	revolver->pos += Vec3(0, .02, 0);
	revolver->rotation = glm::rotate(revolver->rotation, -.3f, Vec3(0, 0, -1));
	revolver->spread += 2.0f;
	revolver->ammo--;
}

void CreateRevolver(Player* player) {
	Model* revolverModel = ModelManagerAllocate(&modelManager, "res/models/revolver.glb");

	player->revolver.renderModel = new RenderModel;
	player->revolver.renderModel->model = revolverModel;
	player->revolver.renderModel->rotation = Quat(1.0, 0, 0, 0);
	player->revolver.renderModel->scale = Vec3(1);
	player->revolver.renderModel->translation = Vec3(0);
	player->revolver.renderModel->pose = (SkeletonPose*)ScratchArenaAllocateZero(&globalArena, sizeof(SkeletonPose));
	player->revolver.renderModel->pose->globalPose = (Mat4*)ScratchArenaAllocateZero(&globalArena, revolverModel->skeleton->numNodes * sizeof(Mat4));
	player->revolver.renderModel->pose->pose = (JointPose*)ScratchArenaAllocateZero(&globalArena, revolverModel->skeleton->numNodes * sizeof(JointPose));
	player->revolver.renderModel->pose->skeleton = revolverModel->skeleton;

	player->revolver.maxMuzzleFlashTime = .1f;
	player->revolver.animTimeScale = 1.0f;

	player->revolver.state = REVOLVER_RELOADING;
	player->revolver.currentAnimation = revolverModel->animations[0];

	player->revolver.baseOffset = Vec3(.5f, -.4f, -1.1f);
	player->revolver.baseRotation = glm::rotate(Quat(1, 0, 0, 0), glm::radians(92.0f), Vec3(0, 1, 0));
	player->revolver.renderModel->scale = Vec3(.4f);

	player->revolver.pos = player->revolver.baseOffset;
	player->revolver.rotation = player->revolver.baseRotation;
	player->revolver.ammo = 6;
	player->revolver.spreadDecayRate = 6.0f;

	player->revolver.Equip = RevolverEquip;
	player->revolver.Update = RevolverUpdate;
	player->revolver.Shoot = RevolverShoot;

}

void ShotgunEquip(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;

}

void ShotgunUpdate(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;
	EntityAnimationUpdate(weapon, dt);

	shotgun->pos = glm::mix(shotgun->pos, shotgun->baseOffset, 5.0f * dt);
	shotgun->rotation = glm::slerp(shotgun->rotation, shotgun->baseRotation, 5.0f * dt);


	if (shotgun->state == SHOTGUN_RELOAD) {
		if (shotgun->currentAnimationPercent == 1.0f) {
			EntityStartAnimation(shotgun, SHOTGUN_ANIM_IDLE);
			shotgun->state = SHOTGUN_IDLE;
			shotgun->mag = 2.0f;
		}
	}
}

static inline void ShotgunShootPellet(Player* player, Shotgun* shotgun, Vec3 start, Vec3 dir, float spread) {
	HitInfo info{};

	float min = -spread;
	float max = spread;

	float x = min + (float)(rand()) / (float(RAND_MAX / (max - min)));
	float y = min + (float)(rand()) / (float(RAND_MAX / (max - min)));
	float z = min + (float)(rand()) / (float(RAND_MAX / (max - min)));
	

	dir += Vec3(x, y, z);
	dir = glm::normalize(dir);
	dir *= 550.0f;

	if (PhysicsQueryRaycast(start, dir, &info)) {
		if (info.entity != 0 && info.entity->OnHit != 0) {
			EntityHitInfo attack{};
			attack.attacker = player;
			attack.victim = info.entity;
			attack.projectile = 0;
			attack.damage = 1;
			info.entity->OnHit(attack);
		}
	}

	//for (int i = 0; i < 100.0f; i++)
	//	DebugDrawAABB(start + glm::normalize(dir) * .3f * (float)i, Vec3(.05f), 10.0f);
	//DebugDrawLine(start, start + dir, GREEN, 1.0f, true, false, 10.0f);
}

void ShotgunAltShoot(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;
	if (shotgun->state == SHOTGUN_RELOAD)
		return;

	shotgun->mag--;

	//Shoot center pellete
	ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, 0);
	//Shoot rest randomly
	for (int i = 1; i < shotgun->numPellets / 3; i++) {
		ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, player->shotgun.spreadRadians / 4.0f);
	}

	if (shotgun->mag == 0.0f) {
		shotgun->state = SHOTGUN_RELOAD;
		EntityStartAnimation(shotgun, SHOTGUN_ANIM_RELOAD);
	}
	
	CreatePlayerMuzzleLight(weapon, player);
	shotgun->pos += Vec3(0, .1, .1);
	shotgun->rotation = glm::rotate(shotgun->rotation, glm::radians(25.0f), Vec3(0, 0, 1));
}

void ShotgunShoot(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;

	if (shotgun->state == SHOTGUN_RELOAD)
		return;

	//do an alt fire if only 1 shot left
	if (shotgun->mag == 1) {
		ShotgunAltShoot(player, weapon);
		return;
	}

	shotgun->state = SHOTGUN_RELOAD;
	EntityStartAnimation(shotgun, SHOTGUN_ANIM_RELOAD);

	//Shoot center pellete
	ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, 0);
	//Shoot rest randomly
	for (int i = 1; i < shotgun->numPellets; i++) {
		ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, player->shotgun.spreadRadians);
	}

	CreatePlayerMuzzleLight(weapon, player);
}


void CreateShotgun(Player* player) {
	Model* shotgunModel = ModelManagerAllocate(&modelManager, "res/models/shotgun.glb");

	player->shotgun.renderModel = new RenderModel;
	player->shotgun.renderModel->model = shotgunModel;
	player->shotgun.renderModel->rotation = Quat(1.0, 0, 0, 0);
	player->shotgun.renderModel->scale = Vec3(1);
	player->shotgun.renderModel->translation = Vec3(0);
	player->shotgun.renderModel->pose = (SkeletonPose*)ScratchArenaAllocateZero(&globalArena, sizeof(SkeletonPose));
	player->shotgun.renderModel->pose->globalPose = (Mat4*)ScratchArenaAllocateZero(&globalArena, shotgunModel->skeleton->numNodes * sizeof(Mat4));
	player->shotgun.renderModel->pose->pose = (JointPose*)ScratchArenaAllocateZero(&globalArena, shotgunModel->skeleton->numNodes * sizeof(JointPose));
	player->shotgun.renderModel->pose->skeleton = shotgunModel->skeleton;

	player->shotgun.maxMuzzleFlashTime = .1f;
	player->shotgun.animTimeScale = 1.0f;

	player->shotgun.state = REVOLVER_RELOADING;
	player->shotgun.currentAnimation = shotgunModel->animations[1];

	player->shotgun.baseOffset = Vec3(.3f, -.4f, -.5f);
	player->shotgun.baseRotation = glm::rotate(Quat(1, 0, 0, 0), glm::radians(91.0f), Vec3(0, 1, 0));
	//player->shotgun.renderModel->scale = Vec3(0.7f,1.0f,1.4f);

	player->shotgun.pos = player->shotgun.baseOffset;
	player->shotgun.rotation = player->shotgun.baseRotation;
	player->shotgun.ammo = 2;
	player->shotgun.mag = 2;
	player->shotgun.spreadDecayRate = 6.0f;

	player->shotgun.numPellets = 16;
	player->shotgun.spreadRadians = glm::radians(15.0f);

	player->shotgun.Equip = ShotgunEquip;
	player->shotgun.Update = ShotgunUpdate;
	player->shotgun.Shoot = ShotgunShoot;
	player->shotgun.AltShoot = ShotgunAltShoot;
}
