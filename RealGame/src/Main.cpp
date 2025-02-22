#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_image.h"
#include <AL/al.h>

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
*	Milestone 3
*	Renderer
*	
*	Lightmap
*		Fix raycast
*			Remove all in map except for single wall & lightmap.
*			get the 1 face that should be able to see it
*			Draw the rays
*			Debug properly
*		UVs wrong
* 
*	Bound Lighting
*		Probably use sphere to make it little amount of extra data
*		If done on CPU with tiled rendering or something, then can try AABB
*		Can probably just solve for 0 using attenuation * intensity
*	
*	Shadows
*		Atlas?
*			Filtering on an atlas
*	Decals
*	
* 
*	.Def
*		Model
*		static Enemy who has default stats
*		This makes it easy to copy and can read from .def file at start

	=====================
			VFX
	=====================
	Enemy Spawn
		3D Model?

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

	Encounters should be placed in map with the name of the encounter text file	
	This way it can be added when compiled but still hotloaded

	==================================
				Gameplay
	==================================
	Triggers
		Delay
		Message that appears on screen
		Sound

	CPU Flipbooks
		Explosion, etc
			These are not particles but just a static image

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

Console console;

Sound explosion;

float dt;
float gameTime = 0;
bool paused = false;

int main() {
	CreateScratchArena( &globalArena, TOTAL_MEMORY, malloc( TOTAL_MEMORY ), NULL, "Global Arena" );
	console.Init();

	CreateStackArena( &tempArena, TEMP_MEMORY, ScratchArenaAllocate( &globalArena, TEMP_MEMORY ), &globalArena, "Temp Arena" );

	WindowInit( &window, 1280, 720, "Game for real this time guys" );
	WindowAddKeySubscription( &window, &console.sub );

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

	CreateEntityManager();
	Wizard::model = ModelManagerAllocate( &modelManager, "res/models/wizardsmooth.glb" );
	Wizard::projectileModel = ModelManagerAllocate( &modelManager, "res/models/WizardBall.glb" );

	LoadWavFile( &explosion, "res/sounds/Explosion.wav" );
	LoadWavFile( &Player::revolverFireSound, "res/sounds/RevolverShoot.wav" );
	LoadWavFile( &Player::revolverReloadSound, "res/sounds/RevolverReload.wav" );
	LoadWavFile( &Wizard::shootSound, "res/sounds/WizardShoot.wav" );
	LoadWavFile( &Wizard::ballExplosionSound, "res/sounds/ballExplode.wav" );
	LoadWavFile( &Wizard::spotSound, "res/sounds/seeWizard.wav" );
	LoadWavFile( &Wizard::deathSound, "res/sounds/dieWizard.wav" );
	LoadWavFile( &Wizard::staggerSound, "res/sounds/stgWizard.wav" );

	LoadWavFile( &Goblin::staggerSound, "res/sounds/StgGoblin.wav" );
	Goblin::model = ModelManagerAllocate( &modelManager, "res/models/goblinsmooth.glb" );

	RegisterCvar( "noclip", ConsoleToggleNoClip, CV_FUNC );
	RegisterCvar( "map", ConsoleChangeLevel, CV_FUNC );

	//Generate Deadpose
	Wizard::deadPose = ( SkeletonPose* ) ScratchArenaAllocate( &globalArena, sizeof( SkeletonPose ) );
	Wizard::deadPose->globalPose = ( Mat4*  ) ScratchArenaAllocate( &globalArena, sizeof(Mat4) * Wizard::model->skeleton->numBones );
	Wizard::deadPose->pose = ( JointPose* ) ScratchArenaAllocate( &globalArena, sizeof( Mat4 ) * Wizard::model->skeleton->numNodes );
	Wizard::deadPose->skeleton = Wizard::model->skeleton;
	AnimatePose( Wizard::model->animations[WIZARD_ANIM_DEATH]->duration - .001f, Wizard::model->animations[WIZARD_ANIM_DEATH], Wizard::deadPose );
	UpdatePose( Wizard::deadPose->skeleton->root, Mat4( 1.0 ), Wizard::deadPose );

	CreateLevel( &level, ScratchArenaAllocate( &globalArena, LEVEL_MEMORY ), LEVEL_MEMORY );
	LoadLevel( &level, "res/maps/lighting.cum" );
	Timer timer;

	Player* player = (Player*) entityManager.player;
	//Gibs
	ModelManagerAllocate( &modelManager, "res/models/gib.glb" );

	//Model
	Goblin::model->animations[GOBLIN_ANIM_RUN]->looping = true;
	Wizard::model->animations[WIZARD_ANIM_RUN]->looping = true;

	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 1 );

	bool start = true;

	renderer.drawStats = true;
	 
	AudioSource* source = NewAudioSource();
	alSourcef( source->alSourceIndex, AL_GAIN, .25f );

	//RenderModel sponza;
	//sponza.model = ModelManagerAllocate( &modelManager, "DevAssets/Models/sponza.glb" );

	bool triggered = false;

	while( !WindowShouldClose( &window ) ) {
		player = entityManager.player;
		//PROFILE( "Frame" );
		xOffset = 0;
		yOffset = 0;
		KeysUpdate();
		WindowPollInput( &window );
		console.Update();

		UpdateSounds();

		if( KeyPressed( KEY_P ) ) {
			//paused = !paused;
		}

		if( KeyPressed( KEY_T ) ) {
			triggered = true;
		}

		if( KeyPressed( KEY_TAB ) ) {
			window.cursorLocked = !window.cursorLocked;
			if( window.cursorLocked ) {
				glfwSetInputMode( window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
			}
			else {
				glfwSetInputMode( window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			}
		}


		float dir[6];
		dir[0] = renderer.camera.Front.x;
		dir[1] = renderer.camera.Front.y;
		dir[2] = renderer.camera.Front.z;

		dir[3] = renderer.camera.Up.x;
		dir[4] = renderer.camera.Up.y;
		dir[5] = renderer.camera.Up.z;

		SoundSetListenerPosition( player->camera.Position );
		alListenerfv( AL_ORIENTATION, dir );

		timer.Tick();
		dt = timer.GetTimeSeconds();

		if( KeyDown( KEY_T ) ) {
			start = true;
			dt = 1.0f / 144.0f;
		}
		if( !start ) continue;

		timer.Restart();

		//if( dt > 5.0f / 144.0f )
		//	dt = 1.0f / 144.0f;

#if 1

#endif
		if( entityManager.numEntities == 2 ) {
			printf( "" );
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

		renderer.camera = player->camera;
		RenderStartFrame( &renderer );
		//RenderDrawModel( &renderer, sponza.model );
		RenderDrawFrame( &renderer, dt );

		{
			//PROFILE( "SKELETAL" );
			//for( int i = 0; i < 250; i++ )
			//	RenderDrawEntity( wizard );
		}

		RenderDrawMuzzleFlash( renderer.blankTexture );

		if( paused )
			RenderDrawText( Vec2( 600, 300 ), 48, "PAUSED" );

		RenderEndFrame( &renderer );
		WindowSwapBuffers( &window );
	}
}
