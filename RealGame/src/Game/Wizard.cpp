#include "wizard.h"
#include "EntityManager.h"
#include "Physics/Physics.h"
#include "Resources/ModelManager.h"
#include "Renderer/DebugRenderer.h"
Model* Wizard::model;

Wizard* CreateWizard( Vec3 pos ) {
	Wizard* wizard = ( Wizard* ) NewEntity();
	CreateBoid( wizard );
	EntityGenerateRenderModel( wizard, Wizard::model, &globalArena );
	wizard->pos = pos;
	wizard->state = 0;
	wizard->health = 1;
	wizard->maxHealth = 5;
	wizard->currentAnimation = Wizard::model->animations[0];
	//wizard->renderModel->scale = Vec3( .45 );

	wizard->bounds->bounds.center = Vec3( 0, 2.8, 0 );
	wizard->bounds->bounds.width = Vec3( 1.55f, 3.2, 1.55f );
	wizard->bounds->bounds.center += Vec3( -.1f, .6f, 0 );
	wizard->bounds->offset = wizard->pos;
	wizard->renderModel->scale = Vec3(3);

	DebugDrawCharacterCollider( wizard->bounds, Vec3(0,1,0), true, true, 10000.0f );

	return wizard;
}