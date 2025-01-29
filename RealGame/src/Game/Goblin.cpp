#include "Game.h"
#include "Resources/ModelManager.h"

Model* Goblin::model = 0;

Goblin* CreateGoblin( Vec3 pos ) {
	//Todo Better Solution
	Goblin* goblin = ( Goblin* ) NewEntity();
	EntityGenerateRenderModel( goblin, Goblin::model, &globalArena );
	goblin->pos = pos;
	goblin->currentAnimation = goblin->renderModel->model->animations[0];
	goblin->state = GOBLIN_CHASE;
	goblin->health = 2;
	goblin->maxHealth = 2;
	goblin->currentAnimation = Goblin::model->animations[0];

	return goblin;
}
	