#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_image.h"

#include "def.h"
#include "Resources\ModelManager.h"
#include "Resources\ShaderManager.h"
#include "Resources\Shader.h"
#include "Resources\TextureManager.h"
#include "Renderer\Renderer.h"
#include "Resources\Level.h"
#include "Renderer\DebugRenderer.h"

#include "Physics\Physics.h"
#include "game/Game.h"

/*
*	Next MVP:

	Proper Entity Resource System?
		ensure that it Only load resources once

	I want a battlefield (Almost like Doom dark ages)
	I want many little goblins running at the player
	I want goblins to explode into gibs when they die
	Gibs should delete after X time (Or when circular buffer gets full?)
		Gibs should NOT every be queried, nor should they ever try to do physics once landed
			MAYBE 1 raycast every second incase floor below them fell down
	1 Shot on goblin should stun
	2 Shots should kill and explode
		should be able to cause a chain reaction


	Quake FGD File
		Player Spawn
		Ogre
		Goblins

	Basic Particle System 
		Blood when enemy shot
		Explosion when enemy dies
			Explosion + Gore
		Muzzle Flash


*
* Animation
*	Animation Events		
*		Should be able to add a new channel for events
*		This may mean it is time to add the decl format.
*	Properly loopign animataions 
*	Figure out where to store hud textures
*	Add a default texture for failing to get them. 
		stop asserting and start warning

	Actual Revolver Spread
*		Look into exponational decay rate, linear to slow

*	Physics:
*		EntityColliders to be able to detect if they intersect each other
*		Im fine if they clip each i think, i can have a collision resolve stage?
* 
* 	Possible Optimizations:
		Sparse List for entities. Right now it loops over all 1000, which shouldn't be 
		too bad however each entity is 2K and that will not be cache coherent at all

	Graphics
*		Map Renderering
*			Cull Brushes with AABB from camera
*				Can probably use the convex hucll BVH for this
*				Dont worry about individual faces, quicker to just cull entire brushes

*/

//Eventually
//Todo STB_Malloc, Realloc, Free
//Todo look into a precompiled header like doom 3 bfg's
//Todo decide how to automate shader args


Window window;
ModelManager modelManager;
Renderer renderer;

ScratchArena globalArena;
StackArena tempArena;

ShaderManager shaderManager;
TextureManager textureManager;

Physics physics;
EntityManager entityManager;

Level level;

float dt;
float gameTime = 0;

int main() {
	CreateScratchArena( &globalArena, TOTAL_MEMORY, malloc( TOTAL_MEMORY ), NULL, "Global Arena" );
	CreateStackArena( &tempArena, TEMP_MEMORY, ScratchArenaAllocate( &globalArena, TEMP_MEMORY ), &globalArena, "Temp Arena" );

	WindowInit( &window, 1280, 720, "Game for real this time guys" );

	CreateModelManager( &modelManager,
		MODEL_MANAGER_SIZE, ScratchArenaAllocate( &globalArena, MODEL_MANAGER_SIZE ),
		ANIMATION_MANAGER_SIZE, ScratchArenaAllocate( &globalArena, ANIMATION_MANAGER_SIZE ) );

	void* shaderMemory = ScratchArenaAllocate( &globalArena, sizeof( ShaderInfo ) * MAX_SHADERS );
	void* argMemory = ScratchArenaAllocate( &globalArena, sizeof( ShaderArg ) * MAX_SHADER_ARGS );
	CreateShaderManager( &shaderManager, MAX_SHADERS, shaderMemory, MAX_SHADER_ARGS, argMemory );
	CreateTextureManager( ScratchArenaAllocate( &globalArena, MAX_TEXTURES * sizeof( Texture ) ), MAX_TEXTURES * sizeof( Texture ) );

	CreateRenderer( &renderer, 0, 0 );
	CreateDebugRenderer( &renderer, ScratchArenaAllocate( &globalArena, DEBUG_RENDERER_SIZE ), DEBUG_RENDERER_SIZE );

	PhysicsInit();
	CreateEntityManager();

	CreateLevel( &level, ScratchArenaAllocate( &globalArena, LEVEL_MEMORY ), LEVEL_MEMORY );
	LoadLevel( &level, "res/maps/battlefield.cum" );
	Timer timer;

	Player* player = CreatePlayer( Vec3( 0 ) );
	player->camera.Yaw = 180.0;
	player->camera.GetViewMatrix();

	//Model
	Goblin::model = ModelManagerAllocate( &modelManager, "res/models/goblin.glb" );
	Goblin::model->animations[0]->looping = true;

	Goblin* goblin = CreateGoblin( Vec3( 5 ) );

#if 0
	Entity* goblin = NewEntity();
	Model* model = ModelManagerAllocate( &modelManager, "res/models/goblin.glb" );
	EntityGenerateRenderModel( goblin, model, &globalArena );
	goblin->pos = player->pos + player->camera.Front * 6.0f;
	goblin->currentAnimation = goblin->renderModel->model->animations[0];
	goblin->currentAnimation->looping = true;
#endif

	for ( int i = 0; i < physics.numBrushes; i++ ) {
		Brush* brush = &physics.brushes[i];
		for ( int n = 0; n < brush->numPolygons; n++ ) {
			Polygon* face = &brush->polygons[n];
			for ( int k = 0; k < face->numTriangles; k++ ) {
				u32* tri = face->triangles[k].v;
				Vec3* verts = brush->vertices;
				//DebugDrawLine( verts[tri[0]], verts[tri[1]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
				//DebugDrawLine( verts[tri[1]], verts[tri[2]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
				//DebugDrawLine( verts[tri[2]], verts[tri[0]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
			}
		}
	}

	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 0 );

	bool start = true;

	renderer.drawStats = true;

	while ( !WindowShouldClose( &window ) ) {
		//PROFILE( "Frame" );
		KeysUpdate();
		WindowPollInput( &window );

		EntityAnimationUpdate( goblin, dt );

		timer.Tick();
		dt = timer.GetTimeSeconds();
		if ( KeyDown( KEY_T ) ) {
			start = true;
			dt = 1.0f / 144.0f;
		}
		if ( !start ) continue;

		timer.Restart();
		gameTime += dt;

		//Movement
		UpdateEntities();

		UpdateProjectiles();
		EntityManagerCleanUp();

		AnimateEntities();

		renderer.camera = player->camera;
		RenderStartFrame( &renderer );
		RenderDrawFrame( &renderer, dt );

		//Draw Projectiles
		for ( int i = 0; i < entityManager.numProjectiles; i++ ) {
			Projectile* projectile = &entityManager.projectiles[i];
			if ( projectile->state != ACTIVE_ACTIVE || projectile->model.model == 0 )
				continue;

			Mat4 t = glm::translate( Mat4( 1.0 ), projectile->collider.offset + projectile->collider.bounds.center );
			RenderDrawModel( &renderer, projectile->model.model, t );
		}

		//Draw Gun
		{
			glClear( GL_DEPTH_BUFFER_BIT );
			Mat4 t = glm::translate( Mat4( 1.0 ), player->revolver.pos );
			Mat4 r = glm::toMat4( player->revolver.rotation );
			Mat4 s = glm::scale( Mat4( 1.0 ), player->revolver.renderModel->scale );
			Mat4 model = t * r * s;
			model = glm::inverse( renderer.camera.GetViewMatrix() ) * model;
			RenderDrawModel( &renderer, entityManager.player->revolver.renderModel->model, model, entityManager.player->revolver.renderModel->pose );
		}

		//Draw Crosshair
		if ( player->revolver.state != REVOLVER_RELOADING ) {
			Vec2 spreadSize(16 * player->revolver.spread);
			RenderDrawQuadTextured( Vec2( 640, 360 ) - spreadSize / 2.0f, spreadSize, renderer.crosshairTex );
		}

		//DrawAmmo
		for ( int i = 0; i < player->revolver.ammo; i++ )
			RenderDrawQuadColored( Vec2( 20 * i + 20, 640 ), Vec2( 10, 20 ), Vec3( 1, .97, .86 ) );
		
		RenderDrawQuadTextured( Vec2( 16, 680 ), Vec2( 32 ), renderer.healthTex );
		RenderDrawHealthBar( Vec2( 64, 680 ), Vec2( 120, 30 ), player->health, player->maxHealth );

		char buffer[2048];
		sprintf_s( buffer, 2048, "ms: %.2f\nfps: %.0f", dt * 1000.0f, 1.0f / dt );
		RenderDrawText( Vec2( 1000, 60 ), 32, buffer );
 
		RenderEndFrame( &renderer );
		WindowSwapBuffers( &window );
	}
}
