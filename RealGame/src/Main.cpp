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
	Smoke Transparencies
		If big spheres of smoke can be all done on CPU

	I want many little goblins running at the player
		should be able to cause a chain reaction

	Quake FGD File
		Player Spawn
		Ogre
		Goblins

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
bool paused = false;

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
	//player->camera.Yaw = 180.0;
	player->camera.GetViewMatrix();

	//Gibs
	ModelManagerAllocate( &modelManager, "res/models/gib.glb" );

	//Model
	Goblin::model = ModelManagerAllocate( &modelManager, "res/models/goblin.glb" );
	Goblin::model->animations[0]->looping = true;
#if 1
	Goblin* goblin = CreateGoblin(Vec3(-48, 1, -28));
	Goblin* goblin2 = CreateGoblin(Vec3(-50, 1, -9));
	Goblin* goblin3 = CreateGoblin(Vec3(-36, 1, 14));

	Goblin* bgoblin = CreateGoblin(Vec3(-24, 1, -28));
	Goblin* bgoblin3 = CreateGoblin(Vec3(-12, 1, 14));
#endif

	PrintAllocators( &globalArena );
	WindowSetVsync( &window,1 );

	bool start = true;

	renderer.drawStats = true;

	while ( !WindowShouldClose( &window ) ) {
		//PROFILE( "Frame" );
		KeysUpdate();
		WindowPollInput( &window );

		if ( KeyPressed( KEY_P ) )
			paused = !paused;

		timer.Tick();
		dt = timer.GetTimeSeconds();

		if ( KeyDown( KEY_T ) ) {
			start = true;
			dt = 1.0f / 144.0f;
		}
		if ( !start ) continue;

		timer.Restart();

		if (dt > 1.0f / 144.0f)
			dt = 1.0f / 144.0f;

		if ( !paused ) {
			gameTime += dt;
			//Movement
			UpdateBoids();
			UpdateEntities();
			UpdateProjectiles();
			PhysicsRigidBodiesUpdate();
			EntityManagerCleanUp();
			AnimateEntities();
		}

		//printf("%.2f %.2f %.2f\n", player->pos.x, player->pos.y, player->pos.z);

		DebugDrawLine( Vec3( 0 ), Vec3( 10 ) );

		
		renderer.camera = player->camera;
		RenderStartFrame( &renderer );
		RenderDrawFrame( &renderer, dt );

		RenderDrawMuzzleFlash(renderer.blankTexture);

		if ( paused )
			RenderDrawText( Vec2( 600, 300 ), 48, "PAUSED" );

		RenderEndFrame( &renderer );
		WindowSwapBuffers( &window );
	}
}
