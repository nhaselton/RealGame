#include "Entity.h"
#include "Resources/ModelManager.h"

void EntityStartAnimation( Entity* entity, int index ) {
	entity->currentAnimation =  entity->renderModel->model->animations[index];
	entity->currentAnimationTime = 0;
	entity->currentAnimationPercent = 0;
}

void EntityAnimationUpdate( Entity* entity, float dt ) {
	if ( !entity->currentAnimation )
		return;

	entity->currentAnimationTime += dt;

	if ( entity->currentAnimationTime > entity->currentAnimation->duration ) {
		if ( entity->currentAnimation->looping )
			entity->currentAnimationTime -= entity->currentAnimation->duration;
		else
			entity->currentAnimationTime = entity->currentAnimation->duration;
	}

	entity->currentAnimationPercent = entity->currentAnimationTime / entity->currentAnimation->duration;

	AnimatePose( entity->currentAnimationTime, entity->currentAnimation, entity->renderModel->pose );
	UpdatePose( entity->renderModel->model->skeleton->root, Mat4( 1.0 ), entity->renderModel->pose );
}

