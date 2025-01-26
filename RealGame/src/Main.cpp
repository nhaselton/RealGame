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

#include "Game\Entity.h"
#include "Game\Ogre.h"
/*
*	Physics:
*		Entities with EntityColliders
*		EntityColliders to be able to detect if they intersect each other
*		Im fine if they clip each i think, i can have a collision resolve stage?
*		
*	MVP
*
*	Guns For Player:
*	Revolver
*		Fires as fast as click
*		6 Shots then reload (Spin 360)
*
*
*	Ogre
*		AI
*			Find Player
*			Walk Around
*
*		Shootable
*		Killable (Sphere per bone?)
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

	CreateLevel( &level, ScratchArenaAllocate( &globalArena, LEVEL_MEMORY ), LEVEL_MEMORY );
	LoadLevel( &level, "res/maps/map.cum" );
	Timer timer;

	Entity player;
	player.pos = Vec3( 0 );
	//player.bounds = PhysicsCreateDynamicBody( &player, Vec3( 0 ), Vec3( .5f, 1.0f, .5f ) );
	//player.bounds = ( BoundsHalfWidth* ) ScratchArenaAllocate( &globalArena, sizeof( BoundsHalfWidth ) );
	//player.bounds->center = Vec3( 0 );
	//player.bounds->width = Vec3( .5f,1.0f,.5f );
	player.renderModel = 0;

	//Model
	Entity* ogre;
	ogre = CreateOgre( Vec3( 13, -5, -20 ), &player );

	//Revolver
	Model* revolver = ModelManagerAllocate( &modelManager, "res/models/revolver.glb" );
	Mat4 revolverScale = glm::scale( Mat4( 1.0 ), Vec3( .25f ) );
	Mat4 rot = glm::toMat4( glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 90.0f ), Vec3( 0, 1, 0 ) ) );

	for ( int i = 0; i < physics.numBrushes; i++ ) {
		Brush* brush = &physics.brushes[i];
		for ( int n = 0; n < brush->numPolygons; n++ ) {
			Polygon* face = &brush->polygons[n];
			for ( int k = 0; k < face->numTriangles; k++ ) {
				u32* tri = face->triangles[k].v;
				Vec3* verts = brush->vertices;
				DebugDrawLine( verts[tri[0]], verts[tri[1]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
				DebugDrawLine( verts[tri[1]], verts[tri[2]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
				DebugDrawLine( verts[tri[2]], verts[tri[0]], Vec3( 0, 0, 1 ), 1.5f, true, false, 10000.0f );
			}
		}
	}


	PrintAllocators( &globalArena );
	WindowSetVsync( &window, 1 );
	while ( !WindowShouldClose( &window ) ) {
		PROFILE( "Frame" );
		WindowPollInput( &window );
		timer.Tick();
		dt = timer.GetTimeSeconds();
		timer.Restart();
		gameTime += dt;

		//Movement
		float mx = 0, my = 0, speed = 15;
		if ( KeyDown( KEY_LEFT ) ) mx -= 3;
		if ( KeyDown( KEY_RIGHT ) ) mx += 3;
		if ( KeyDown( KEY_DOWN ) ) my -= 1.5;
		if ( KeyDown( KEY_UP ) ) my += 1.5;
		renderer.camera.ProcessMouseMovement( mx * speed * dt * 50, my * dt * speed * 50 );

		Vec3 wantDir( 0 );
		if ( KeyDown( KEY_W ) ) wantDir += renderer.camera.Front;
		if ( KeyDown( KEY_S ) ) wantDir -= renderer.camera.Front;
		if ( KeyDown( KEY_A ) ) wantDir += -renderer.camera.Right;
		if ( KeyDown( KEY_D ) ) wantDir += renderer.camera.Right;
		if ( wantDir != Vec3( 0 ) ) 
			wantDir = glm::normalize( wantDir );

		//OgreUpdate( ogre );
		//PlayerMovement( &player, wantDir * 20.0f * dt);
		HitInfo info{};

		//for ( int i = 0; i < 1000; i++ )
		wantDir *= 20.0f * dt;
		//wantDir.y = 0;
		Vec3 r( .5, 1, .5f );
		player.pos = MoveAndSlide( player.pos, wantDir, r);

		if ( KeyDown(KEY_SPACE) )
			player.pos += wantDir ;
		else
			MoveAndSlide( renderer.camera.Position, renderer.camera.Front * 10.0f, r );
		
		renderer.camera.Position = player.pos + Vec3( 0, 1, 0 );
		RenderStartFrame( &renderer );
		RenderDrawFrame( &renderer, dt );
		RenderDrawEntity( ogre );
		{
			glClear( GL_DEPTH_BUFFER_BIT );
			Mat4 t = glm::translate( Mat4( 1.0 ), Vec3( 3, -2.5, -1.2 ) );
			Mat4 r = glm::toMat4( glm::rotate( Quat( 1, 0, 0, 0 ), glm::radians( 80.0f ), Vec3( 0, 1, 0 ) ) );
			Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( .4f ) );
			Mat4 model = t * r * s;
			model = glm::inverse( renderer.camera.GetViewMatrix() ) * model;
			RenderDrawModel( &renderer, revolver, model );
		}


		RenderEndFrame( &renderer );

		WindowSwapBuffers( &window );
	}
}
