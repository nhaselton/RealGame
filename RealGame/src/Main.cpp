#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_image.h"

#include "def.h"
#include "Resources\ModelManager.h"
#include "Resources\ShaderManager.h"
#include "Resources\Shader.h"
#include "Resources\SoundManager.h"
#include "Resources\TextureManager.h"
#include "Resources\Level.h"

#include "Renderer\Renderer.h"
#include "Renderer\DebugRenderer.h"

#include "Physics\Physics.h"
#include "game/Game.h"
/*
* 
* ======================
*	Triggers
* ===================
	Trigger Type to Spawn Single Enemy
		Enemy Type
		SpawnPoint
	Trigger Type to Spawn Multiple Enemies
		Types of enemies
		SpawnGroup (Only 1) 
		Count

	=====================
			AI
	=====================
	Dont walk off edges
	IDLE state until they see / hear player
	Wizard require LOS
	Polish Wizard
		Projecitle model
		Sounds
	Polish Goblin
		Sounds

	===================
			Game
	===================
	Pistol shooting should hit static geo

	====================
			Rendering
	====================
	Super basic directional Lighting
		Small ambient + directional light
		fix rotated skeletal meshes not lighting properly

	=====================
			Sounds
	=====================
	Wizard Notice Player
	Wizard Pain 
	Wizard Shoot
	Wizard Orb Explode

	Goblin Notice Player
	Goblin Pain

	Goblin Scream
	Footsteps
	Music
	Spawn Sound


	=====================
			VFX
	=====================
	Enemy Spawn

	=====================
			Art
	=====================
	Wizard Orb Model

	MVP:
		Single 2 or 3 room level (Start of karnack pretty much)
			that starts in small room with Ragned guys
			Goes outdoors into field that has kamakazi and ranged guys


	==================================
		Whats Next
	==================================
	.def files
	Encounter files / GUI
		One File for all encounters
		could generate a .cpp file?
			But then no hot reloading

	Fix Spawn Loading Code
	Its super hacked in.

	CPU Flipbooks
		Explosion, etc
			These are not particles but just a static image

	Console:
		Commands:
			noclip
			showtriggers
			drawphysics

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
*		Customizable Boid settings (avoidance, interest in target etc.)
*			Combine this with AABB collisions and it should be swag
*
* 	Possible Optimizations:
		Sparse List for entities. Right now it loops over all 1000, which shouldn't be
		too bad however each entity is 2K and that will not be cache coherent at all

	Graphics
*		Map Renderering
*			Cull Brushes with AABB from camera
*				Can probably use the convex hucll BVH for this
*				Dont worry about individual faces, quicker to just cull entire brushes

*	Finish Paritcles
*		3) Indirect Drawing
*			Have particles write num particles alive to new buffer and draw indirect it
*		4) Fadeout over time

*/

//Eventually
//Todo STB_Malloc, Realloc, Free
//Todo look into a precompiled header like doom 3 bfg's
//Todo decide how to automate shader args
//Fxi dead body not replacing perfectly


Window window;
ModelManager modelManager;
Renderer renderer;

SoundManager soundManager;

ScratchArena globalArena;
StackArena tempArena;

ShaderManager shaderManager;
TextureManager textureManager;

Physics physics;
EntityManager entityManager;
Level level;

Sound explosion;

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

	CreateSoundManager();
	//CreateSoundSystem ( &soundDevice, &soundContext );
	Sound sound{};
	LoadWavFile( &sound, "res/sounds/RevolverShoot.wav" );
	LoadWavFile( &explosion, "res/sounds/Explosion.wav" );

	CreateEntityManager();
	CreateEncounters();

	Wizard::model = ModelManagerAllocate( &modelManager, "res/models/wizard.glb" );
	Goblin::model = ModelManagerAllocate( &modelManager, "res/models/goblin.glb" );

	//Generate Deadpose
	Wizard::deadPose = ( SkeletonPose* ) ScratchArenaAllocate( &globalArena, sizeof( SkeletonPose ) );
	Wizard::deadPose->globalPose = ( Mat4*  ) ScratchArenaAllocate( &globalArena, sizeof(Mat4) * Wizard::model->skeleton->numBones );
	Wizard::deadPose->pose = ( JointPose* ) ScratchArenaAllocate( &globalArena, sizeof( Mat4 ) * Wizard::model->skeleton->numNodes );
	Wizard::deadPose->skeleton = Wizard::model->skeleton;
	AnimatePose( Wizard::model->animations[WIZARD_ANIM_DEATH]->duration - .001f, Wizard::model->animations[WIZARD_ANIM_DEATH], Wizard::deadPose );
	UpdatePose( Wizard::deadPose->skeleton->root, Mat4( 1.0 ), Wizard::deadPose );

	CreateLevel( &level, ScratchArenaAllocate( &globalArena, LEVEL_MEMORY ), LEVEL_MEMORY );
	LoadLevel( &level, "res/maps/demo.cum" );
	Timer timer;

	Player* player = (Player*) entityManager.player;
	//Gibs
	ModelManagerAllocate( &modelManager, "res/models/gib.glb" );

	//Model
	Goblin::model->animations[GOBLIN_ANIM_RUN]->looping = true;
	Wizard::model->animations[WIZARD_ANIM_RUN]->looping = true;

	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 0 );

	bool start = true;

	renderer.drawStats = true;

	AudioSource* source = NewAudioSource();

	bool triggered = false;
	while( !WindowShouldClose( &window ) ) {
		//PROFILE( "Frame" );
		KeysUpdate();
		WindowPollInput( &window );
		UpdateSounds();

		if( KeyPressed( KEY_P ) ) {
			paused = !paused;
		}

		if( KeyPressed( KEY_T ) ) {
			triggered = true;
		}

		if( KeyPressed( KEY_SPACE ) ) {
			PlaySound( source, &sound );
		}

		timer.Tick();
		dt = timer.GetTimeSeconds();

		if( KeyDown( KEY_T ) ) {
			start = true;
			dt = 1.0f / 144.0f;
		}
		if( !start ) continue;

		timer.Restart();

		if( dt > 1.0f / 144.0f )
			dt = 1.0f / 144.0f;

		for( int i = 0; i < entityManager.numTriggers; i++ ) {
			Trigger* trigger = &entityManager.triggers[i];
			//DebugDrawBoundsMinMax( &entityManager.triggers[i].bounds, RED,0,false );
			DebugDrawAABB( trigger->bounds.min, Vec3( .75f ), 0, GREEN );
			DebugDrawAABB( trigger->bounds.max, Vec3( .75f ), 0, GREEN );

			Vec3 center = ( trigger->bounds.min + trigger->bounds.max ) / 2.0f;
			Vec3 size = ( trigger->bounds.max - trigger->bounds.min ) / 2.0f;
			DebugDrawAABB( center, size );
		}

		for( int i = 0; i < entityManager.numEncounters; i++ ) {
			Encounter& encounter = entityManager.encounters[i];
			if( encounter.active )
				UpdateEncounter( &encounter );
		}


		if( !paused ) {
			gameTime += dt;
			//Movement
			UpdateBoids();
			UpdateEntities();
			UpdateProjectiles();
			PhysicsRigidBodiesUpdate();
			EntityManagerCleanUp();
			AnimateEntities();
		}
		char buffer[2048]{};
		sprintf_s( buffer, 2048, "Player pos: %.2f %.2f %.2f\n", player->pos.x, player->pos.y, player->pos.z );
		RenderDrawText( Vec2( 0, 360 ), 16, buffer );

		DebugDrawLine( Vec3( 0 ), Vec3( 10 ) );

		renderer.camera = player->camera;
		RenderStartFrame( &renderer );
		RenderDrawFrame( &renderer, dt );

		RenderDrawMuzzleFlash( renderer.blankTexture );

		if( paused )
			RenderDrawText( Vec2( 600, 300 ), 48, "PAUSED" );

		RenderEndFrame( &renderer );
		WindowSwapBuffers( &window );
	}
}
