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

	//===============================
			 Demo (Done By Doom)
	//===============================

	//====================
	//Milestone 1
	//		Gameplay (April 1st)
	//===================
	Proper Equip/Unequip Weapon Functions
		Quickly pull them up with small delay? shouldnt be able to insta quickswap
		Animation Cancels

	Enemies (Types)
		Done
			Explosive Goblin	
				Make Blow up when Close
			Chaingunner (Threat at distance)
				More bullets per shoot event & add tiny bit of spread to them
			Small Ogre (Annoyance/Body block)
				Remove entity at death
			Bull 
				Polish all parts
					Try Keeping velocity frame by frame and just capping magnitude
					Would no longer need to check if running wrong way
					Note: Would always try to run in ent->forward direction adn slowly turn
				MAYBE: Fireball Wizard (Fires in arc?)
					Different color same model?

	Balance enemy health values
	Balance enemy stagger
		Could do float staggerPercent and make damage contribute to it  but have it decay over time so fast damage is required for bigger things

	Level:
		10 Minutes long
		Small Rooms to show off less enemies
		Large open fields for lots of enemies
		4 way split at end to get 4 pieces to unlock exit?
	
	Triggers
		Delay
		Message that appears on screen
		Sound

*	Properly looping animataions
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

	EntityManagerCleanUp might be causing crashes
	//====================
	//  Milestone 2
	//		Visuals
	//====================
	By May 9
	Shadows
		Either shadow map or do little shadows underneath
			Could probably draw a sphere and if it intersects things it draws black
	VFX
	Enemy Spawn
		3D Model?

	CPU Flipbooks
		Explosion, etc
			These are not particles but just a static image
*
	Graphics
*		Map Renderering
*			Cull Brushes with AABB from camera
*				Can probably use the convex hucll BVH for this
*				Dont worry about individual faces, quicker to just cull entire brushes

	*	Light:
	*		Have Game ignore static lights? Create staic option in TB
	*		Type //Separate arrays? Can upload static once and dynamic per frame
	*			Static
	*			Dynamic
	*		Calculate Radius
	*			Solve quadratic for when light < .1f?
	*		Shape
	*			Point
	*			Spot
	*			Directional
	*		Shadows: Can create when level loads if close enough to it? Unload when too far?
	*			Static
	*			All
	*			None
	*
	*	Bound Lighting
	*		Probably use sphere to make it little amount of extra data
	*		If done on CPU with tiled rendering or something, then can try AABB
	*		Can probably just solve for 0 using attenuation * intensity

	*	Finish Paritcles
	*		3) Indirect Drawing
	*			Have particles write num particles alive to new buffer and draw indirect it
	*		4) Fadeout over time
	* 
	//====================
		Milestone 3 Polish
	//====================
	//Sounds
		Guns
			Pistol
				Sounds
					Equip
			Shotgun
				Sounds
					Primary
					Secondary
					Reload
					Equip
			PlasmaGun
				Sounds
					Shoot
					Ambient hum?
					Equip
				Rocket Launcher
					Sounds:
						Equip
						Fire
						Reload
		Enemies
			Chaingunner
				Sounds:
					Shoot
					Stagger
					Die
			Boar


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
int maxFps;
int sleepTime;

void LoadDecls() {
	Wizard::model = LoadModel("res/def/wizard.def");
	Chaingunner::model = LoadModel("res/def/chaingunner.def");
	Boar::model = LoadModel("res/models/boar.glb");

}

int main() {
	CreateScratchArena( &globalArena, TOTAL_MEMORY, malloc( TOTAL_MEMORY ), NULL, "Global Arena" );
	console.Init();
	CreateStackArena( &tempArena, TEMP_MEMORY, ScratchArenaAllocate( &globalArena, TEMP_MEMORY ), &globalArena, "Temp Arena" );

	WindowInit( &window, 1920, 1080, "Game for real this time guys" );
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

	LoadDecls();

	//Wizard::model = ModelManagerAllocate( &modelManager, "res/models/wizardsmooth.glb" );
	Wizard::projectileModel = ModelManagerAllocate( &modelManager, "res/models/WizardBall.glb" );

	RegisterCvar( "startencounter", ConsoleStartEncounter, CV_FUNC );
	RegisterCvar( "maxfps", &maxFps, CV_INT );
	RegisterCvar( "reloadencounters", ConsoleReloadEncounterFile, CV_FUNC );
	RegisterCvar( "reloaddecls", LoadDecls, CV_FUNC );
	RegisterCvar( "killai", KillAI, CV_FUNC );
	RegisterCvar( "noclip", ConsoleToggleNoClip, CV_FUNC );
	RegisterCvar( "map", ConsoleChangeLevel, CV_FUNC );

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

	Chaingunner::projectileModel = ModelManagerAllocate(&modelManager, "res/models/CGBullet.glb");

	Ogre::model = ModelManagerAllocate( &modelManager, "res/models/ogre.glb" );
	Ogre::projectileModel = ModelManagerAllocate( &modelManager, "res/models/rock.glb" );

	//Generate Deadpose
	Wizard::deadPose = ( SkeletonPose* ) ScratchArenaAllocate( &globalArena, sizeof( SkeletonPose ) );
	Wizard::deadPose->globalPose = ( Mat4*  ) ScratchArenaAllocate( &globalArena, sizeof(Mat4) * Wizard::model->skeleton->numBones );
	Wizard::deadPose->pose = ( JointPose* ) ScratchArenaAllocate( &globalArena, sizeof( Mat4 ) * Wizard::model->skeleton->numNodes );
	Wizard::deadPose->skeleton = Wizard::model->skeleton;
	AnimatePose( Wizard::model->animations[WIZARD_ANIM_DEATH]->duration - .001f, Wizard::model->animations[WIZARD_ANIM_DEATH], Wizard::deadPose );
	UpdatePose( Wizard::deadPose->skeleton->root, Mat4( 1.0 ), Wizard::deadPose );

#if 0
	Chaingunner::deadPose = (SkeletonPose*)ScratchArenaAllocate(&globalArena, sizeof(SkeletonPose));
	Chaingunner::deadPose->globalPose = (Mat4*)ScratchArenaAllocate(&globalArena, sizeof(Mat4) * Chaingunner::model->skeleton->numBones);
	Chaingunner::deadPose->pose = (JointPose*)ScratchArenaAllocate(&globalArena, sizeof(Mat4) * Chaingunner::model->skeleton->numNodes);
	Chaingunner::deadPose->skeleton = Chaingunner::model->skeleton;
	AnimatePose(Chaingunner::model->animations[CG_ANIM_DEATH]->duration - .001f, Chaingunner::model->animations[CG_ANIM_DEATH], Chaingunner::deadPose);
	UpdatePose(Chaingunner::deadPose->skeleton->root, Mat4(1.0), Chaingunner::deadPose);
#endif

	CreateLevel( &level, ScratchArenaAllocate( &globalArena, LEVEL_MEMORY ), LEVEL_MEMORY );
	LoadLevel( &level, "res/maps/demo.cum" );
	Timer timer;

	Player* player = (Player*) entityManager.player;
	//Gibs
	ModelManagerAllocate( &modelManager, "res/models/gib.glb" );

	//Model
	Goblin::model->animations[GOBLIN_ANIM_RUN]->looping = true;
	Wizard::model->animations[WIZARD_ANIM_RUN]->looping = true;
	Chaingunner::model->animations[CG_ANIM_SHOOT]->looping = true;
	Chaingunner::model->animations[CG_ANIM_IDLE]->looping = true;
	Chaingunner::model->animations[CG_ANIM_RUN]->looping = true;
	Boar::model->animations[BOAR_ANIM_CHARGE]->looping = true;
	//Boar::model->animations[BOAR_ANIM_IDLE]->looping = true;

	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 0 );

	bool start = true;

	renderer.drawStats = true;
	 
	AudioSource* source = NewAudioSource();
	alSourcef( source->alSourceIndex, AL_GAIN, .25f );

	bool triggered = false;
	maxFps = 250;
	//Boar* boar = CreateBoar(Vec3(0, 1, 0));
	while( !WindowShouldClose( &window ) ) {
		if( maxFps > 0 )
			NSpinLock( maxFps );


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
