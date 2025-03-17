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

void RevolverUpdate(Player* player, Weapon* weapon) {
	Revolver* revolver = (Revolver*)weapon;
	EntityAnimationUpdate(revolver, dt * 1.25f);

	if( revolver->state == REVOLVER_EQUIPPING ) {
		revolver->pos += Vec3( 0, 3, 0 ) * dt;
		if( revolver->pos.y >= revolver->baseOffset.y ) {
			revolver->state = REVOLVER_IDLE;
		}
		return;
	}

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
	if (revolver->state == REVOLVER_RELOADING || revolver->state == REVOLVER_EQUIPPING)
		return;

	//Try to shoot with no ammo in clip
	if (revolver->ammo == 0) {
		PlaySound(player->audioSource, &Player::revolverReloadSound);
		revolver->state = REVOLVER_RELOADING;
		EntityStartAnimation(revolver, REVOLVER_ANIM_RELOAD);
		return;
	}

	if (revolver->currentShootCooldown > gameTime)
		return;

	revolver->currentShootCooldown = gameTime + revolver->shootCooldown;

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

void RevolverAltShoot(Player* player, Weapon* weapon) {
	//(Hack) Check if the ammo is lower to see if the gun shot properly. 
	// If so then we can remove the shoot cooldown & add extra spread
	int ammo = player->revolver.ammo;
	RevolverShoot(player, weapon);

	if (ammo != player->revolver.ammo) {
		player->revolver.spread += 1.0f;
		player->revolver.currentShootCooldown = 0.0f;
	}
}

void RevolverEquip( Player* player, Weapon* weapon ) {
	player->currentWeapon = weapon;
	player->revolver.pos = player->revolver.baseOffset - Vec3( 0,1.0f,0 );
	player->revolver.state = REVOLVER_EQUIPPING;
	player->revolver.ammo = 6;
	player->revolver.spread = 0;
	player->revolver.rotation = player->revolver.baseRotation;
	player->revolver.currentAnimationTime = 1.0f;
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

	player->revolver.state = REVOLVER_IDLE;
	player->revolver.currentAnimation = revolverModel->animations[REVOLVER_ANIM_SHOOT];

	player->revolver.baseOffset = Vec3(.5f, -.4f, -1.1f);
	player->revolver.baseRotation = glm::rotate(Quat(1, 0, 0, 0), glm::radians(92.0f), Vec3(0, 1, 0));
	player->revolver.renderModel->scale = Vec3(.4f);

	player->revolver.pos = player->revolver.baseOffset;
	player->revolver.rotation = player->revolver.baseRotation;
	player->revolver.ammo = 6;
	player->revolver.spreadDecayRate = 6.0f;

	player->revolver.shootCooldown = .4f;
	player->revolver.fastShootCooldown = .16f;

	player->revolver.Equip = RevolverEquip;
	player->revolver.Update = RevolverUpdate;
	player->revolver.Shoot = RevolverShoot;
	player->revolver.AltShoot = RevolverAltShoot;
	player->revolver.Equip = RevolverEquip;
}

void ShotgunEquip(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;
	player->currentWeapon = shotgun;
	shotgun->state = SHOTGUN_EQUIPPING;
	shotgun->mag = 2;
	shotgun->pos = shotgun->baseOffset - Vec3( 0, 1.0f, 0 );
	player->revolver.rotation = player->revolver.baseRotation;
	EntityStartAnimation( shotgun, SHOTGUN_ANIM_IDLE );
}

void ShotgunUpdate(Player* player, Weapon* weapon) {
	Shotgun* shotgun = (Shotgun*)weapon;

	if( shotgun->state == SHOTGUN_EQUIPPING ) {
		if( shotgun->pos.y >= shotgun->baseOffset.y ) {
			shotgun->state = SHOTGUN_IDLE;
		}
		shotgun->pos.y += 3 * dt;
		return;
	}

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
	else {
		if (shotgun->mag < 2 && KeyDown(KEY_R)) {
			EntityStartAnimation(shotgun, SHOTGUN_ANIM_RELOAD);
			shotgun->state = SHOTGUN_RELOAD;
		}
			

	}
}

static inline void ShotgunShootPellet(Player* player, Shotgun* shotgun, Vec3 start, Vec3 dir, float spread) {
	HitInfo info{};

	float min = -spread;
	float max = spread;

	float x = min + (float)(rand()) / (float(RAND_MAX / (max - min)));

	//make it spread slightly less on the Y axis
	max /= 2.0f;
	min /= 2.0f;
	float y = min + (float)(rand()) / (float(RAND_MAX / (max - min)));

	dir = dir + x * player->camera.Right + y * player->camera.Up;
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
	if( shotgun->state == SHOTGUN_RELOAD || shotgun->state == SHOTGUN_EQUIPPING )
		return;

	shotgun->mag--;

	//Shoot center pellete
	ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, 0);
	//Shoot rest randomly
	for (int i = 1; i < shotgun->numPellets / 3; i++) {
		ShotgunShootPellet(player, shotgun, player->camera.Position, player->camera.Front, player->shotgun.spreadRadians / 2.0f);
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

	if( shotgun->state == SHOTGUN_RELOAD || shotgun->state == SHOTGUN_EQUIPPING )
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

	player->shotgun.numPellets = 16;//Try to keep (count - 1) % 3 == 0 for secondary fire to have exactly 1/3rd the amount of pelettes
	player->shotgun.spreadRadians = glm::radians(15.0f);

	player->shotgun.Equip = ShotgunEquip;
	player->shotgun.Update = ShotgunUpdate;
	player->shotgun.Shoot = ShotgunShoot;
	player->shotgun.AltShoot = ShotgunAltShoot;
}

void PlasmaGunUpdate(Player* player, Weapon* weapon) {
	EntityAnimationUpdate(weapon, dt);
	PlasmaGun* plasma = (PlasmaGun*) weapon;
	if( weapon->state == PLASMA_EQUIPPING ) {
		if( plasma->pos.y >= plasma->baseOffset.y ) {
			plasma->state = PLASMA_READY;
		}
		plasma->pos.y += 3 * dt;
		return;
	}

	weapon->pos = glm::mix(weapon->pos, weapon->baseOffset, 5.0f * dt);
	weapon->rotation = glm::slerp(weapon->rotation, weapon->baseRotation, 5.0f * dt);
}

void PlasmaGunShoot(Player* player, Weapon* weapon) {
	PlasmaGun* pg = (PlasmaGun*)weapon;
	if( pg->state == PLASMA_EQUIPPING ) return;

	if (gameTime - pg->currentCooldown >= 0) {
		pg->currentCooldown = gameTime + pg->shotCooldown;

		Vec3 start = player->camera.Position;
		start += Vec3(0, -.8, 0);
		start += player->camera.Front * 2.5f;
		start += player->camera.Right * 0.7f;

		pg->pos += Vec3(0, .1, .1);
		pg->rotation = glm::rotate(pg->rotation, glm::radians(2.0f), Vec3(0, 0, 1));

		Projectile* orb = NewProjectile(start, player->camera.Front * 40.0f, Vec3(.5f), true);
		if (orb) {
			orb->collider.owner = player;
			orb->model.model = Wizard::projectileModel;
			orb->model.scale = Vec3(.5f);
			orb->model.translation = Vec3(0);
			orb->OnCollision = WizardBallCallback;
			//PlaySound( wizard->audioSource, &Wizard::shootSound );
		}
	}
}

void PlasmaGunAltShoot(Player* player, Weapon* weapon) {

}

void PlasmaGunEquip(Player* player, Weapon* weapon) {
	player->currentWeapon = weapon;
	PlasmaGun* pg = (PlasmaGun*) weapon;
	pg->pos = pg->baseOffset + Vec3( 0, -1, 0 );
	pg->state = PLASMA_EQUIPPING;
}

void CreatePlasmaGun(class Player* player) {
	Model* plasmaModel = ModelManagerAllocate(&modelManager, "res/models/plasma.glb");
	player->plasmaGun.renderModel = new RenderModel;
	player->plasmaGun.renderModel->model = plasmaModel;
	player->plasmaGun.renderModel->rotation = Quat(1.0, 0, 0, 0);
	player->plasmaGun.renderModel->translation = Vec3(0);
	player->plasmaGun.renderModel->pose = (SkeletonPose*)ScratchArenaAllocateZero(&globalArena, sizeof(SkeletonPose));
	player->plasmaGun.renderModel->pose->globalPose = (Mat4*)ScratchArenaAllocateZero(&globalArena, plasmaModel->skeleton->numNodes * sizeof(Mat4));
	player->plasmaGun.renderModel->pose->pose = (JointPose*)ScratchArenaAllocateZero(&globalArena, plasmaModel->skeleton->numNodes * sizeof(JointPose));
	player->plasmaGun.renderModel->pose->skeleton = plasmaModel->skeleton;

	player->plasmaGun.currentAnimation = 0;
	player->plasmaGun.renderModel->scale = Vec3(.6f);
	player->plasmaGun.baseOffset = Vec3(.5f, -.6f, -.3);
	player->plasmaGun.baseRotation = glm::rotate(Quat(1, 0, 0, 0), glm::radians(96.0f), Vec3(0, 1, 0));

	player->plasmaGun.pos = player->plasmaGun.baseOffset;
	player->plasmaGun.rotation = player->plasmaGun.baseRotation;

	player->plasmaGun.shotCooldown = 1.0f / 10.0f;
	player->plasmaGun.currentCooldown = 0.0f;
	player->plasmaGun.fullAuto = true;
	
	player->plasmaGun.Equip = PlasmaGunEquip;
	player->plasmaGun.Update = PlasmaGunUpdate;
	player->plasmaGun.Shoot = PlasmaGunShoot;
	player->plasmaGun.AltShoot = PlasmaGunAltShoot;
}

void RocketLauncherEquip(Player* player, Weapon* weapon) {
	RocketLauncher* rpg = (RocketLauncher*) weapon;

	player->currentWeapon = rpg;
	rpg->pos = rpg->baseOffset + Vec3( 0, -3, 0 );
	EntityStartAnimation( rpg, RL_ANIM_IDLE );
};

void RocketLauncherUpdate( Player* player, Weapon* weapon){
	RocketLauncher* rpg = (RocketLauncher*)weapon;

	if( rpg->state == RL_EQUIPPING) {
		rpg->pos += Vec3( 0, 3, 0 ) * dt;
		if( rpg->pos.y >= rpg->baseOffset.y ) {
			rpg->state = RL_READY;
		}
		return;
	}

	EntityAnimationUpdate(weapon, dt);

	if (rpg->state == RL_FIRE) {
		if (rpg->currentAnimationPercent == 1.0f) {
			rpg->state = RL_READY;
			EntityStartAnimation(rpg, RL_ANIM_IDLE);
		}
	}

	rpg->pos = glm::mix(rpg->pos, rpg->baseOffset, 5.0f * dt);
	rpg->rotation = glm::slerp(rpg->rotation, rpg->baseRotation, 5.0f * dt);
};

void RocketLauncherShoot( Player* player, Weapon* weapon){
	RocketLauncher* rpg = (RocketLauncher*)weapon;

	if (rpg->state != RL_READY)
		return;

	rpg->state = RL_FIRE;
	EntityStartAnimation(rpg, RL_ANIM_FIRE);


	Vec3 start = player->camera.Position;
	start += Vec3(0, -.8, 0);
	start += player->camera.Front * 2.5f;
	start += player->camera.Right * 0.7f;

	rpg->rotation = glm::rotate(rpg->rotation, glm::radians(2.0f), Vec3(0, 0, 1));
	Projectile* orb = NewProjectile(start, player->camera.Front * 40.0f, Vec3(.5f), true);
	if (orb) {
		orb->collider.owner = player;
		orb->model.model = Wizard::projectileModel;
		orb->model.scale = Vec3(.5f);
		orb->model.translation = Vec3(0);
		orb->OnCollision = RocketCallback;
		//PlaySound( wizard->audioSource, &Wizard::shootSound );
	}

};

void RocketLauncherAltShoot( Player* player, Weapon* weapon){

};

void CreateRocketLauncher(class Player* player) {
	Model* rocketModel = ModelManagerAllocate(&modelManager, "res/models/RPG.glb");
	player->rocketLauncher.renderModel = new RenderModel;
	player->rocketLauncher.renderModel->model = rocketModel;
	player->rocketLauncher.renderModel->rotation = Quat(1.0, 0, 0, 0);
	player->rocketLauncher.renderModel->translation = Vec3(0);
	player->rocketLauncher.renderModel->pose = (SkeletonPose*)ScratchArenaAllocateZero(&globalArena, sizeof(SkeletonPose));
	player->rocketLauncher.renderModel->pose->globalPose = (Mat4*)ScratchArenaAllocateZero(&globalArena, rocketModel->skeleton->numNodes * sizeof(Mat4));
	player->rocketLauncher.renderModel->pose->pose = (JointPose*)ScratchArenaAllocateZero(&globalArena, rocketModel->skeleton->numNodes * sizeof(JointPose));
	player->rocketLauncher.renderModel->pose->skeleton = rocketModel->skeleton;

	player->rocketLauncher.renderModel->scale = Vec3(.6f);
	player->rocketLauncher.baseOffset = Vec3(.5f, -.6f, -.3);
	player->rocketLauncher.baseRotation = glm::rotate(Quat(1, 0, 0, 0), glm::radians(96.0f), Vec3(0, 1, 0));

	player->rocketLauncher.pos = player->plasmaGun.baseOffset;
	player->rocketLauncher.rotation = player->plasmaGun.baseRotation;

	player->rocketLauncher.state = RL_READY;
	player->rocketLauncher.animTimeScale = 1.0f;
	EntityStartAnimation(&player->rocketLauncher, RL_ANIM_IDLE);

	player->rocketLauncher.Equip = RocketLauncherEquip;
	player->rocketLauncher.Update = RocketLauncherUpdate;
	player->rocketLauncher.Shoot = RocketLauncherShoot;
	player->rocketLauncher.AltShoot = RocketLauncherAltShoot;
}

void RocketCallback(Projectile* projectile, Entity* entity) {
	TEMP_ARENA_SET;
	Entity** list = (Entity**)TEMP_ALLOC(sizeof(void*) * 32);

	//Get last tick so out of wall
	Vec3 pos = projectile->collider.offset - projectile->velocity * dt;
	int numHit = PhysicsQueryExplosion(pos, 12.0f, list, 32);
	DebugDrawSphere(pos, 12.0f,Vec3(1,0,0),true,1,1,1.0f);

	DebugDrawAABB(pos, projectile->collider.bounds.width, 1000.0f);
	for (int i = 0; i < numHit; i++) {
		if (list[i]->OnHit) {
			EntityHitInfo info{};
			info.attacker = projectile->owner;
			info.victim = list[i];
			info.projectile = projectile;
			info.damage = 100;
			list[i]->OnHit(info);
		}
	}

	RemoveProjectile(projectile);
}