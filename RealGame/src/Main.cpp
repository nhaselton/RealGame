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
	Defaults for when a file cant be read:
		Models
		Textures

	Shaders
		or togther types of shaders
		pass const char* [] for paths, will know count by going through probably
		Change all Shader* stuff in renderer to shader_t to make it less tedious
	Shader Reloading.
		Will just have to re find args for all arguments? 
	
	Come back to particles this shit sucks

	Navmesh?
		Enemys are circle on navmesh, shoot rays around the navmesh to see what they hit?
	

	I want many little goblins running at the player
	I want goblins to explode into gibs when they die
	Gibs should delete after X time (Or when circular buffer gets full?)
		Gibs should NOT every be queried, nor should they ever try to do physics once landed
			MAYBE 1 raycast every second incase floor below them fell down
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
	player->camera.Yaw = 180.0;
	player->camera.GetViewMatrix();

	//Gibs
	ModelManagerAllocate( &modelManager, "res/models/gib.glb" );

	//Model
	Goblin::model = ModelManagerAllocate( &modelManager, "res/models/goblin.glb" );
	Goblin::model->animations[0]->looping = true;
	Goblin* goblin = CreateGoblin( Vec3( 0 ) );

	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 1 );

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
		
		if ( !paused ) {
			gameTime += dt;
			//Movement
			UpdateEntities();
			UpdateProjectiles();
			PhysicsRigidBodiesUpdate();
			EntityManagerCleanUp();
			AnimateEntities();
		}

		DebugDrawLine( Vec3( 0 ), Vec3( 10 ) );

		renderer.camera = player->camera;
		RenderStartFrame( &renderer );
		RenderDrawFrame( &renderer, dt );

		if ( paused )
			RenderDrawText( Vec2( 600, 300 ), 48, "PAUSED" );

		RenderEndFrame( &renderer );
		WindowSwapBuffers( &window );
	}
}
