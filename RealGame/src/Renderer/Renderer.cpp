#include <glm/gtx/euler_angles.hpp>
#include "Renderer.h"
#include "Resources/ShaderManager.h"
#include "Resources/Shader.h"
#include "Resources\ModelManager.h"
#include "Game/EntityManager.h"
#include "Renderer\DebugRenderer.h"
#include "Resources\TextureManager.h"
#include "resources/Level.h"
#include "Game/Entity.h"
#include "Game/Player.h"
#include "Physics\Physics.h"

extern ModelManager modelManager;

void RenderCreateShaders( Renderer* renderer ) {
	//Brute force here. Set nons at end
	//XYZRGB
	renderer->shaders[SHADER_XYZRGB] = ShaderManagerCreateShader( &shaderManager, "res/shaders/xyzrgb/xyzrgb.vert", "res/shaders/xyzrgb/xyzrgb.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_XYZRGB] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_XYZRGB], SHADER_ARG_VEC3, "color" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_XYZRGB], SHADER_ARG_MAT4, "model" );

	//XYZRGB Skinned
	renderer->shaders[SHADER_XYZRGB_SKINNED] = ShaderManagerCreateShader( &shaderManager, "res/shaders/xyzrgbskinned/xyzrgb.vert", "res/shaders/xyzrgbskinned/xyzrgb.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_XYZRGB_SKINNED] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_XYZRGB_SKINNED], SHADER_ARG_VEC3, "color" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_XYZRGB_SKINNED], SHADER_ARG_MAT4_ARRAY, "bones" );
	ShaderSetMat4Array( renderer, renderer->shaders[SHADER_XYZRGB_SKINNED], "bones", renderer->mat4Array, 100 );

	//Standrard
	renderer->shaders[SHADER_STANDARD] = ShaderManagerCreateShader( &shaderManager, "res/shaders/standard/standard.vert", "res/shaders/standard/standard.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD], SHADER_ARG_INT, "albedo" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD], SHADER_ARG_INT, "fullbright" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD], "fullbright", renderer->fullBright );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD], SHADER_ARG_MAT4, "model" );

	//Placement
	renderer->shaders[SHADER_PLACE] = ShaderManagerCreateShader( &shaderManager, "res/shaders/place/place.vert", "res/shaders/place/place.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_PLACE] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_PLACE], SHADER_ARG_INT, "albedo" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_PLACE], SHADER_ARG_INT, "fullbright" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_PLACE], "fullbright", renderer->fullBright );
	ShaderSetInt( renderer, renderer->shaders[SHADER_PLACE], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_PLACE], SHADER_ARG_MAT4, "model" );

	//Standard Geo (Lightmap)
	renderer->shaders[SHADER_STANDARD_GEO] = ShaderManagerCreateShader( &shaderManager, "res/shaders/standardGeo/standardGeo.vert", "res/shaders/standardGeo/standardGeo.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD_GEO] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_GEO], SHADER_ARG_INT, "albedo" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_GEO], SHADER_ARG_INT, "fullbright" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_GEO], "fullbright", renderer->fullBright );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_GEO], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_GEO], SHADER_ARG_INT, "lightmap" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_GEO], "lightmap", S2D_LIGHTMAP );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_GEO], SHADER_ARG_MAT4, "model" );


	//Standrard Skinned
	renderer->shaders[SHADER_STANDARD_SKINNED] = ShaderManagerCreateShader( &shaderManager, "res/shaders/standardskinned/standardskinned.vert", "res/shaders/standardskinned/standardskinned.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD_SKINNED] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_INT, "albedo" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_SKINNED], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_MAT4_ARRAY, "bones" );
	ShaderSetMat4Array( renderer, renderer->shaders[SHADER_STANDARD_SKINNED], "bones", renderer->mat4Array, 100 );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_MAT4, "model" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_INT, "fullbright" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_SKINNED], "fullbright", renderer->fullBright );

	//Line
	renderer->shaders[SHADER_LINE_SHADER] = ShaderManagerCreateShader( &shaderManager, "res/shaders/line/line.vert", "res/shaders/line/line.frag" );

	//UI
	renderer->shaders[SHADER_UI] = ShaderManagerCreateShader( &shaderManager, "res/shaders/ui/ui.vert", "res/shaders/ui/ui.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_UI] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_INT, "albedo" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_INT, "solidColor" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_VEC3, "color" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_MAT4, "projection");
	ShaderSetMat4( renderer, renderer->shaders[SHADER_UI], "projection", renderer->orthographic );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_MAT4, "model" );
	ShaderSetMat4( renderer, renderer->shaders[SHADER_UI], "model", Mat4( 1.0 ) );

	ShaderSetInt( renderer, renderer->shaders[SHADER_UI], "albedo", S2D_ALBEDO );
	ShaderSetInt( renderer, renderer->shaders[SHADER_UI], "solidColor", true );
	ShaderSetVec3( renderer, renderer->shaders[SHADER_UI], "color", Vec3( 1, 0, 0 ) );

	renderer->shaders[SHADER_SKYBOX] = ShaderManagerCreateShader( &shaderManager, "res/shaders/skybox/skybox.vert", "res/shaders/skybox/skybox.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_SKYBOX] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_SKYBOX], SHADER_ARG_INT, "albedo" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_SKYBOX], "albedo", S3D_SKYBOX );

	renderer->shaders[SHADER_PARTICLES] = ShaderManagerCreateShader( &shaderManager, "res/shaders/particles/particles.vert", "res/shaders/particles/particles.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_PARTICLES] );

	renderer->shaders[SHADER_COMP_CREATE_PARTICLES] = ShaderManagerCreateComputeShader( &shaderManager, "res/shaders/particles/createParticles.comp" );
	RenderSetShader( renderer, renderer->shaders[SHADER_COMP_CREATE_PARTICLES] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_COMP_CREATE_PARTICLES], SHADER_ARG_FLOAT, "dt" );

	renderer->shaders[SHADER_COMP_UPDATE_PARTICLES] = ShaderManagerCreateComputeShader( &shaderManager, "res/shaders/particles/UpdateParticles.comp" );
	RenderSetShader( renderer, renderer->shaders[SHADER_COMP_UPDATE_PARTICLES] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_COMP_UPDATE_PARTICLES], SHADER_ARG_FLOAT, "dt" );

	renderer->shaders[SHADER_PARTICLES2] = ShaderManagerCreateShader( &shaderManager, "res/shaders/particles2/particles.vert", "res/shaders/particles2/particles.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_PARTICLES2] );
	ShaderAddArg ( &shaderManager, renderer->shaders[SHADER_PARTICLES2], SHADER_ARG_INT, "albedo" );
	ShaderSetInt ( renderer, renderer->shaders[SHADER_PARTICLES2], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_PARTICLES2], SHADER_ARG_MAT4, "model" );

	renderer->shaders[SHADER_COMP_CREATE_PARTICLES2] = ShaderManagerCreateComputeShader( &shaderManager, "res/shaders/particles2/createParticles.comp" );
	RenderSetShader( renderer, renderer->shaders[SHADER_COMP_CREATE_PARTICLES2] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_COMP_CREATE_PARTICLES2], SHADER_ARG_FLOAT, "dt" );

	renderer->shaders[SHADER_COMP_UPDATE_PARTICLES2] = ShaderManagerCreateComputeShader( &shaderManager, "res/shaders/particles2/UpdateParticles.comp" );
	RenderSetShader( renderer, renderer->shaders[SHADER_COMP_UPDATE_PARTICLES2] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_COMP_UPDATE_PARTICLES2], SHADER_ARG_FLOAT, "dt" );

	renderer->shaders[SHADER_BILLBOARD] = ShaderManagerCreateShader( &shaderManager, "res/shaders/billboard/billboard.vert", "res/shaders/billboard/billboard.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_BILLBOARD] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_BILLBOARD], SHADER_ARG_INT, "albedo" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_BILLBOARD], "albedo", S3D_SKYBOX );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_BILLBOARD], SHADER_ARG_MAT4, "model" );

	//Standrard Skinned
	renderer->shaders[SHADER_CELL_SHADER_SKINNED] = ShaderManagerCreateShader(&shaderManager, "res/shaders/CellShaderSkinned/CellShaderSkinned.vert", "res/shaders/CellShaderSkinned/CellShaderSkinned.frag");
	RenderSetShader(renderer, renderer->shaders[SHADER_CELL_SHADER_SKINNED]);
	ShaderAddArg(&shaderManager, renderer->shaders[SHADER_CELL_SHADER_SKINNED], SHADER_ARG_INT, "albedo");
	ShaderSetInt(renderer, renderer->shaders[SHADER_CELL_SHADER_SKINNED], "albedo", S2D_ALBEDO);
	ShaderAddArg(&shaderManager, renderer->shaders[SHADER_CELL_SHADER_SKINNED], SHADER_ARG_MAT4_ARRAY, "bones");
	ShaderSetMat4Array(renderer, renderer->shaders[SHADER_CELL_SHADER_SKINNED], "bones", renderer->mat4Array, 100);
	ShaderAddArg(&shaderManager, renderer->shaders[SHADER_CELL_SHADER_SKINNED], SHADER_ARG_MAT4, "model");
	ShaderAddArg(&shaderManager, renderer->shaders[SHADER_CELL_SHADER_SKINNED], SHADER_ARG_INT, "fullbright");
	ShaderSetInt(renderer, renderer->shaders[SHADER_CELL_SHADER_SKINNED], "fullbright", renderer->fullBright);

	Shader* ui = renderer->shaders[SHADER_UI];
	RenderSetShader( renderer, ui );
	ui->updateMVP = false;
}

void ConsoleFullBright() {
	renderer.fullBright = !renderer.fullBright;
	RenderSetShader( &renderer, renderer.shaders[SHADER_STANDARD] );
	ShaderSetInt( &renderer, renderer.shaders[SHADER_STANDARD], "fullbright", renderer.fullBright );

	RenderSetShader( &renderer, renderer.shaders[SHADER_STANDARD_SKINNED] );
	ShaderSetInt( &renderer, renderer.shaders[SHADER_STANDARD_SKINNED], "fullbright", renderer.fullBright );
}

void CreateRenderer( Renderer* renderer, void* memory, u32 size ) {
	Mat4 view( 1.0 );
	renderer->projection = glm::perspective( glm::radians( 90.0f ), 16.0f / 9.0f, .1f, 2048.0f );
	renderer->orthographic = glm::ortho( 0.0f, ( float )1280.0f, ( float )720.0f, 0.0f, -1.0f, 1.0f );
	Vec4 color( 1, 0, 0, 1 );

	renderer->cube = ModelManagerAllocate( &modelManager, "res/models/cube.glb" );
	renderer->sphere = ModelManagerAllocate( &modelManager, "res/models/sphere.glb" );
	renderer->whiteNoiseTex = TextureManagerLoadTextureFromFile( "res/textures/whitenoise.png" );
	renderer->blankTexture = TextureManagerLoadTextureFromFile( "res/textures/blank.png" );
	renderer->muzzleFlash = TextureManagerLoadTextureFromFile( "res/textures/muzzleFlash.png" );
	renderer->particleAtlas = TextureManagerLoadTextureFromFile ( "res/textures/particleAtlas.png" );
	float quadVertices[] = {
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	};

	CreateGLBuffer( &renderer->quadBuffer, 20, 0, sizeof( quadVertices ), quadVertices, 0, 0, true, true );
	GLBufferAddAttribute( &renderer->quadBuffer, 0, 3, GL_FLOAT, 5 * sizeof( float ), 0 );
	GLBufferAddAttribute( &renderer->quadBuffer, 1, 2, GL_FLOAT, 5 * sizeof( float ), ( void* )(3 * sizeof( float )) );

	RenderCreateShaders( renderer );
	RenderInitFont();

	renderer->worldView.projection = renderer->projection;
	renderer->worldView.view = Mat4( 1.0 );

	renderer->crosshairTex = TextureManagerLoadTextureFromFile("res/textures/crosshair.png");
	renderer->shotgunCrosshairTex = TextureManagerLoadTextureFromFile( "res/textures/ShotgunCrosshair.png" );
	renderer->healthTex = TextureManagerLoadTextureFromFile( "res/textures/health.png" );

	void* lightMemory = ScratchArenaAllocateZero( &globalArena, MAX_DYNAMIC_LIGHTS * sizeof( LightNode ));
	CreatePoolArena( &renderer->lightArena, sizeof( LightNode ), MAX_DYNAMIC_LIGHTS, lightMemory, &globalArena, "Light");

	glGenBuffers( 1, &renderer->particleSSBO2 );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer->particleSSBO2 );
	glBufferData( GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * PARTICLE_SIZE_GPU, 0, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, renderer->particleSSBO2 );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	glGenBuffers( 1, &renderer->particleEmitterSSBO2 );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer->particleEmitterSSBO2 );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( renderer->emitters ), 0, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, renderer->particleEmitterSSBO2 );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	glGenBuffers ( 1, &renderer->particleSortSSBO );
	glBindBuffer ( GL_SHADER_STORAGE_BUFFER, renderer->particleSortSSBO );
	glBufferData ( GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof ( u32 ), 0, GL_DYNAMIC_DRAW );
	glBindBufferBase ( GL_SHADER_STORAGE_BUFFER, 5, renderer->particleSortSSBO );
	glBindBuffer ( GL_SHADER_STORAGE_BUFFER, 0 );

	//glGenBuffers( 1, &renderer->boneSSBO );
	//glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer->boneSSBO );
	//glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( Mat4 ) * MAX_BONES * MAX_ENTITIES, 0, GL_DYNAMIC_DRAW );
	//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 7, renderer->boneSSBO );
	//glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	glGenBuffers( 1, &renderer->worldViewSSBO );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer->worldViewSSBO );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( WorldView ), 0, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 8, renderer->worldViewSSBO );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	//Billboard
	float veritcesarr[]{
	0, -1, 1, 0, 1, 11111,
	0, -1, -1, 0, 0, 1111,
	0, 1, -1, 1, 0, 1111,

	0, -1, 1, 0, 1, 1111,
	0, 1, -1, 1, 0, 1111,
	0, 1, 1, 1, 1, 1111
	};

	CreateGLBuffer( &renderer->billboard, 0, 0, sizeof( veritcesarr ), veritcesarr, 0, 0, true, 0 );
	GLBufferAddAttribute( &renderer->billboard, 0, 3, GL_FLOAT, 6 * sizeof( float ), ( void* )0 );
	GLBufferAddAttribute( &renderer->billboard, 1, 3, GL_FLOAT, 6 * sizeof( float ), ( void* )sizeof( Vec3 ) );


	//Create Skybox
	{
		const char* paths[6] = {
			"res/textures/skybox/right.png",
			"res/textures/skybox/left.png",
			"res/textures/skybox/top.png",
			"res/textures/skybox/bottom.png",
			"res/textures/skybox/back.png",
			"res/textures/skybox/front.png",
		};
		//Buffer
		glEnable( GL_DEPTH_TEST );

		glGenTextures( 1, &renderer->skybox.textureID );
		glBindTexture( GL_TEXTURE_CUBE_MAP, renderer->skybox.textureID );
		for (int i = 0; i < 6; i++) {
			Texture* texture = &renderer->skybox.faces[i];
			u8* data = ( u8* )stbi_load( paths[i], &texture->width, &texture->height, &texture->channels, 0 );
			if (!data) {
				LOG_ASSERT( LGS_RENDERER, "Could not load skybox path %s\n", paths[i] );
				return;
			}

			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);

			stbi_image_free( data );
		}
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

		glActiveTexture( GL_TEXTURE0 + S3D_SKYBOX );
		glBindTexture( GL_TEXTURE_CUBE_MAP, renderer->skybox.textureID );

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f,-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,-1.0f, -1.0f, -1.0f,-1.0f,  1.0f, -1.0f,-1.0f,  1.0f, -1.0f,-1.0f,  1.0f,  1.0f,-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,-1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f,-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,-1.0f,  1.0f,  1.0f,-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,-1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,-1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
		};

		CreateGLBuffer( &renderer->skybox.buffer, 36, 0, sizeof( skyboxVertices ), skyboxVertices, 0, 0, true, true );
		GLBufferAddAttribute( &renderer->skybox.buffer, 0, 3, GL_FLOAT, 3 * sizeof( float ), ( void* )0 );
	}

	//Set Up Emitters
	for (int i = 0; i < MAX_PARTICLE_EMITTERS; i++) {
		renderer->emitters.freeList[i] = &renderer->emitters.emitters[i];
	}

	RegisterCvar( "drawstats", &renderer->drawStats, CV_INT );
	RegisterCvar( "drawtriggers", &renderer->drawTriggers, CV_INT );
	RegisterCvar( "fullbright", ConsoleFullBright, CV_FUNC );
}

void RenderInitFont() {
	renderer.fontTex = TextureManagerLoadTextureFromFile( "res/textures/font.png" );
	RenderLoadFontFromFile();

	TEMP_ARENA_SET;
	u32* indices = ( u32* )TEMP_ALLOC( FONT_BATCH_SIZE * sizeof( indices[0] ) * 6 );
	int offset = 0;
	for (int i = 0; i < FONT_BATCH_SIZE * 4; i += 6) {
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}

	CreateGLBuffer( &renderer.fontBuffer, FONT_BATCH_SIZE * 4, FONT_BATCH_SIZE * 6,
		FONT_BATCH_SIZE * 4 * sizeof( FontVert ), renderer.glyphs, FONT_BATCH_SIZE * 6 * sizeof( u32 ),
		indices,
		false, true );
	GLBufferAddAttribute( &renderer.fontBuffer, 0, 2, GL_FLOAT, 4 * sizeof( float ), 0 ); //pos
	GLBufferAddAttribute( &renderer.fontBuffer, 1, 2, GL_FLOAT, 4 * sizeof( float ), ( void* )(2 * sizeof( float )) ); //tex
}

void RenderStartFrame( Renderer* renderer ) {
	if (KeyPressed( KEY_ENTER ) && !console.IsOpen())
		ReloadShaders();

	//Clear frame info
	renderer->currentFrameInfo = (renderer->currentFrameInfo + 1) % MAX_FRAME_INFOS;
	memset( &renderer->frameInfos[renderer->currentFrameInfo], 0, sizeof( FrameInfo ) );

	glClear( GL_DEPTH_BUFFER_BIT );
}

void RenderDrawLevel( Renderer* renderer ) {
	TEMP_ARENA_SET;

	TextureChain* textureChains = renderer->levelInfo.textureChains;
	u32 totalIndices = 0;

	//Frist Loop through and get counts of each surface
	for (u32 i = 0; i < renderer->levelInfo.numFaces; i++) {
		RenderBrushFace* face = &renderer->levelInfo.faces[i];
		TextureChain* chain = &textureChains[face->textureIndex];

		totalIndices += face->numIndices;
		//Add each triangle
		for (int n = 0; n < face->numIndices; n += 3)
			chain->firstIndexOfTriangles[chain->numTriangles++] = face->firstIndex + n;
	}

	//Now Loop through again and construct the index ptr[]
	u32* indices = ( u32* )TEMP_ALLOC( totalIndices * sizeof( u32 ) );
	u32 offset = 0;
	for (int i = 0; i < renderer->levelInfo.numTextures; i++) {
		TextureChain* chain = &textureChains[i];

		for (int n = 0; n < chain->numTriangles; n++) {
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 0];
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 1];
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 2];
		}
	}

	//Upload indices to GPU
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD_GEO] );
	nglBindVertexArray( renderer->levelInfo.buffer.vao );
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->levelInfo.buffer.ebo );
	nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, totalIndices * sizeof( u32 ), indices );
	ShaderSetMat4( renderer, renderer->shaders[SHADER_STANDARD_GEO], "model", Mat4( 1.0 ) );

	offset = 0;
	nglActiveTexture( GL_TEXTURE0 );
	for (int i = 0; i < renderer->levelInfo.numTextures; i++) {
		TextureChain* chain = &textureChains[i];

		if (chain->numTriangles == 0)
			continue;

		if (!chain->texture)
			nglBindTexture( GL_TEXTURE_2D, 0 );
		else
			nglBindTexture( GL_TEXTURE_2D, chain->texture->id );

		//Go back through buffer and do 1 draw call per surface
		nglDrawElements( GL_TRIANGLES, chain->numTriangles * 3, GL_UNSIGNED_INT, ( void* )(offset * sizeof( u32 )) );

		offset += chain->numTriangles * 3;
	}

}

void RenderDrawQuadColored( Vec2 pos, Vec2 size, Vec3 color ) {
	Shader* shader = renderer.shaders[SHADER_UI];
	RenderSetShader( &renderer, shader );

	Mat4 t = glm::translate( Mat4( 1.0 ), Vec3( pos, 0 ) );
	Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( size, 1.0f ) );
	Mat4 trs = t * s;

	ShaderSetMat4( &renderer, shader, "model", trs );
	ShaderSetInt( &renderer, shader, "solidColor", true );
	ShaderSetVec3( &renderer, shader, "color", color );

	nglBindVertexArray( renderer.quadBuffer.vao );
	nglDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void RenderDrawQuadTextured( Vec2 pos, Vec2 size, Texture* texture ) {
	Shader* shader = renderer.shaders[SHADER_UI];
	RenderSetShader( &renderer, shader );

	Mat4 t = glm::translate( Mat4( 1.0 ), Vec3( pos, 0 ) );
	Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( size, 1.0f ) );
	Mat4 trs = t * s;

	ShaderSetMat4( &renderer, shader, "model", trs );
	ShaderSetInt( &renderer, shader, "solidColor", false );

	if (texture) {
		nglActiveTexture( GL_TEXTURE0 );
		nglBindTexture( GL_TEXTURE_2D, texture->id );
	}

	nglBindVertexArray( renderer.quadBuffer.vao );
	nglDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void RenderGatherLights() {
	int i = 0;
	for( LightNode* light = renderer.lightHead; light != 0; light = light->next ) {
		renderer.worldView.dynamicLights[i++] = light->light;
	}
	renderer.worldView.numDynamicLights = i;
}

void RenderDrawFrame( Renderer* renderer, float dt ) {
	RenderGatherLights();

	//reset texture chains
	for (int i = 0; i < renderer->levelInfo.numTextures; i++) {
		renderer->levelInfo.textureChains[i].numTriangles = 0;
	}

	for (int i = 0; i < renderer->worldView.numStaticLights; i++)
		DebugDrawAABB(renderer->worldView.staticLights[i].pos, Vec3(1));

	renderer->worldView.projection = renderer->projection;
	renderer->worldView.view = renderer->camera.GetViewMatrix();
	renderer->worldView.gameTime = gameTime;
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer->worldViewSSBO );
	int worldViewSize = offsetof( WorldView, dynamicLights );//this is size before light[]
	glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, worldViewSize + sizeof( Light ) * renderer->worldView.numDynamicLights, &renderer->worldView );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	RenderDrawTriggers();
	RenderDrawLevel( renderer );
	RenderDrawAllEntities();
	RenderDrawAllProjectiles();
	RenderDrawAllRigidBodies();
	RenderDrawAllPickups();

	DebugRendererFrame( renderer->camera.GetViewMatrix(), renderer->projection, dt );
	//RenderDrawText( Vec2( 0,300 ), 32.0f, "The Quick Brown Fox Jumped Over The Lazy\nSleeping Dog" );
	//RenderDrawFontBatch();
	RenderUpdateAndDrawParticles();

	//Draw Skybox
	glDepthFunc( GL_LEQUAL );
	RenderSetShader( renderer, renderer->shaders[SHADER_SKYBOX] );
	nglBindVertexArray( renderer->skybox.buffer.vao );
	nglDrawArrays( GL_TRIANGLES, 0, 36 );
	glDepthFunc( GL_LESS );

	RenderDrawConsole();


}

void RenderDrawTriggers() {
	if( !renderer.drawTriggers ) {
		//return;
	}

	for( int i = 0; i < entityManager.numTriggers; i++ ) {
		Trigger* trigger = &entityManager.triggers[i];
		Vec3 center = ( trigger->bounds.min + trigger->bounds.max ) / 2.0f;
		Vec3 size = ( trigger->bounds.max - trigger->bounds.min ) / 2.0f;
		DebugDrawAABB( center, size );
	}
}

void RenderUpdateAndDrawParticles() {
	//Debug Draw Emitters
	RenderSetShader( &renderer, renderer.shaders[SHADER_UI] );
	float particleSize = 720.0f / ( float )MAX_PARTICLES;

	glDepthFunc( GL_LEQUAL );

	Vec2 start( 40, 40 );
	RenderDrawQuadColored( start, Vec2( particleSize * ( float )MAX_PARTICLES, 40 ), BLACK );

	Vec3 colors[]{
		RED,
		BLUE,
		GREEN,
		Vec3( 1,1,0 ),
		Vec3( 1,0,1 ),
		Vec3( 0,1,1 ),
	};
	
	for (int i = 0; i < renderer.numEmitters; i++) {
		ParticleEmitter2* emitter = renderer.emitters.activeList[i];
		float particleLength = ( float )emitter->maxParticles / ( float )MAX_PARTICLES * 720.0f;
		int colorIndex = i % (sizeof( colors ) / sizeof( colors[0] ));
		RenderDrawQuadColored( start, Vec2( particleLength, 40 ), colors[colorIndex] );
		start.x += particleLength;
	}
	//Figure out how many particles each must do

	// ==============
	//	Update And Draw Particles
	// ==============

	// Emitter Compute
	RenderSetShader( &renderer, renderer.shaders[SHADER_COMP_CREATE_PARTICLES2] );

	//Figure out how many particles to draw for each
	int totalEmit = 0;
	int firstParticle = 0;
	for (int i = 0; i < renderer.numEmitters; i++) {
		ParticleEmitter2* emitter = renderer.emitters.activeList[i];
		if (emitter->currentEmitterLifetime > emitter->maxEmitterLifeTime)
			continue;
		
		int emit = 0;
		int particleCountNow = 0;

		if (emitter->emitterSpawnType == EMITTER_INSTANT) {
		//	//Shoot all out at once
			if (emitter->currentEmitterLifetime == 0.0f) {
				emit = emitter->maxParticles;
				emitter->numParticles = emit;
			}
			emitter->currentEmitterLifetime += dt;
			particleCountNow = emitter->numParticles;
		}
		else {
			//Figure out how far along the lifetime we are and how many more to add
			emitter->currentEmitterLifetime += dt;
			particleCountNow = ( int )roundf ( emitter->currentEmitterLifetime * emitter->spawnRate );
			emit = particleCountNow - emitter->numParticles;
		}
		
		//Add to proper buffers
		renderer.emitters.numParticlesPerEmitter[i] = emit;
		emitter->numParticles = particleCountNow;
		emitter->particleOffset = firstParticle;
		firstParticle += emitter->maxParticles;
		totalEmit += emit;

		//If this is now empty, remove it after rendering
		if (emitter->currentEmitterLifetime > emitter->maxEmitterLifeTime) {
			renderer.emittersDirty = true;
			continue;
		}
	}

	//This is bad that it has to be recreated each frame, but ill worry about that later
	TEMP_ARENA_SET;
	Renderer::Emitter2* uploadEmitter = (Renderer::Emitter2*) TEMP_ALLOC ( sizeof(Renderer::Emitter2) );
	memcpy ( uploadEmitter->numParticlesPerEmitter, renderer.emitters.numParticlesPerEmitter, MAX_PARTICLE_EMITTERS * 4 );
	for (int i = 0; i < renderer.numEmitters; i++)
		uploadEmitter->emitters[i] = *renderer.emitters.activeList[i];

	//Upload all emitters (Todo only do when dirtydds)
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer.particleEmitterSSBO2 );
	glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( renderer.emitters ), uploadEmitter );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	//Emit Particles
	glDispatchCompute( totalEmit, 1, 1 ); //Creates particles
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, renderer.particleSSBO2 );
	RenderSetShader( &renderer, renderer.shaders[SHADER_COMP_UPDATE_PARTICLES2] );
	ShaderSetFloat( &renderer, renderer.shaders[SHADER_COMP_UPDATE_PARTICLES2], "dt", dt );

	glDispatchCompute( MAX_PARTICLES / 32, 1, 1 ); //updates particles
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
	
	//Draw Particles
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D, renderer.particleAtlas->id );
	RenderSetShader( &renderer, renderer.shaders[SHADER_PARTICLES2] );
	//FOR NOW DO ALL PARTICLES
	glDrawArrays ( GL_TRIANGLES, 0, MAX_PARTICLES * 6 );

	glDisable ( GL_BLEND );

	for (int i = 0; i < renderer.numEmitters; i++) {
		ParticleEmitter2* emitter = renderer.emitters.activeList[i];
		if (emitter->maxEmitterLifeTime == 0 || emitter->currentEmitterLifetime > emitter->maxEmitterLifeTime) {

			//Subtract particleSize from all future emitters
			int moveParticles = 0;
			for (int n = i + 1; n < renderer.numEmitters; n++) {
				ParticleEmitter2* future = &renderer.emitters.emitters[n];
				future->particleOffset -= emitter->numParticles;
				moveParticles += future->numParticles;
			}

			//Copy all Emitters over by one
			for (int n = i; n < renderer.numEmitters - 1; n++) {
				renderer.emitters.activeList[n] = renderer.emitters.activeList[n + 1];
			}

			//Todo Copy buffer data
			glBindBuffer ( GL_COPY_READ_BUFFER, renderer.particleEmitterSSBO2 );
			glBindBuffer ( GL_COPY_WRITE_BUFFER, renderer.particleEmitterSSBO2 );
			int read = (emitter->particleOffset + emitter->numParticles) * PARTICLE_SIZE_GPU;
			int write = (emitter->particleOffset) * PARTICLE_SIZE_GPU;
			glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, read, write, moveParticles * PARTICLE_SIZE_GPU );
			glBindBuffer ( GL_COPY_READ_BUFFER, 0 );
			glBindBuffer ( GL_COPY_WRITE_BUFFER, 0 );

			renderer.numEmitters--;
		}
	}
}

void RenderEndFrame( Renderer* renderer ) {
	char buffer[8192]{};
	if (renderer->drawStats) {
		FrameInfo* info = &renderer->frameInfos[renderer->currentFrameInfo];
		sprintf_s( buffer, 8192, "Draw Calls: %d\nTriangles %d\nShader Binds: %d\nTexture Binds: %d",
			info->drawArrayCalls + info->drawElementCalls,
			(info->drawArrayTriangleCount + info->drawElementTriangleCount) / 2, info->shaderBinds, info->bindTextureCalls );

		RenderDrawText( Vec2( 60, 60 ), 16, buffer );
		RenderDrawFontBatch();
	}

	//Draw UI
	Player* player = entityManager.player;
	for (int i = 0; i < player->revolver.ammo; i++)
		RenderDrawQuadColored( Vec2( 20 * i + 20, 640 ), Vec2( 10, 20 ), Vec3( 1, .97, .86 ) );

	RenderDrawQuadTextured( Vec2( 16, 680 ), Vec2( 32 ), renderer->healthTex );
	RenderDrawHealthBar( Vec2( 64, 680 ), Vec2( 120, 30 ), player->health, player->maxHealth );

	memset( buffer, 0, 8192 );
	sprintf_s( buffer, 2048, "ms: %.2f\nfps: %.0f", dt * 1000.0f, 1.0f / dt );
	RenderDrawText( Vec2( 1000, 60 ), 32, buffer );

	//Crosshairs
	if (player->currentWeapon == &player->revolver) {
		if (player->revolver.state != REVOLVER_RELOADING) {
			Vec2 spreadSize(16 * player->revolver.spread);
			RenderDrawQuadTextured(Vec2(640, 360) - spreadSize / 2.0f, spreadSize, renderer->crosshairTex);
		}
	}
	else if (player->currentWeapon == &player->shotgun) {
		Vec2 size(128, 64);
		RenderDrawQuadTextured(Vec2(640, 360) - size / 2.0f, size, renderer->shotgunCrosshairTex);

	}

	//Draw gun last (Will mess up post processing later on)
	RenderDrawGun();
	RenderDrawFontBatch();
}

void RenderSetShader( Renderer* renderer, Shader* newShader ) {
	if (!newShader || newShader->id == -1) {
		LOG_ASSERT( LGS_RENDERER, "Trying to use shader that does not exist" );
		return;
	}
	//if ( newShader->id == renderer->currentShaderID ) {
	//	LOG_WARNING( LGS_RENDERER, "Setting active shader to same shader\n" );
	//}
	renderer->frameInfos[renderer->currentFrameInfo].shaderBinds++;

	glUseProgram( newShader->id );
	renderer->currentShaderID = newShader->id;
}

void RenderDrawModel( Renderer* renderer, Model* model, Mat4 offset, SkeletonPose* pose, Shader* shaderOverride ) {
	//Shader* shader = (model->skeleton == 0) ? renderer->shaders[SHADER_STANDARD] : renderer->shaders[SHADER_STANDARD_SKINNED];
	Shader* shader = 0;
	if( !shaderOverride )
		shader = ( model->skeleton == 0 ) ? renderer->shaders[SHADER_STANDARD] : renderer->shaders[SHADER_CELL_SHADER_SKINNED];
	else
		shader = shaderOverride;

	RenderSetShader( renderer, shader );

	if (model->skeleton && pose) {
		ShaderSetMat4Array( renderer, shader, "bones", pose->globalPose, model->skeleton->numBones );
	}

	ShaderSetMat4( renderer, shader, "model", offset );

	for (u32 i = 0; i < model->numMeshes; i++) {
		if (model->meshes[i].texture != 0) {
			nglActiveTexture( GL_TEXTURE0 + S2D_ALBEDO );
			nglBindTexture( GL_TEXTURE_2D, model->meshes[i].texture->id );
		}

		nglBindVertexArray( model->meshes[i].buffer.vao );
		nglDrawElements( GL_TRIANGLES, model->meshes[i].numIndices, GL_UNSIGNED_INT, ( void* )0 );

	}
}

void RenderDrawEntity( Entity* entity ) {
	if (entity->renderModel == 0)
		return;

	Mat4 t = glm::translate( Mat4( 1.0 ), entity->renderModel->translation + entity->pos );
	Mat4 rmr = glm::toMat4( entity->renderModel->rotation );
	Mat4 entr = glm::toMat4( entity->rotation );
	Mat4 r = entr * rmr;
	Mat4 s = glm::scale( Mat4( 1.0 ), entity->renderModel->scale );
	Mat4 trs = t * r * s;

	SkeletonPose* pose = (entity->renderModel->pose != 0) ? entity->renderModel->pose : 0;
	RenderDrawModel( &renderer, entity->renderModel->model, trs, pose );
}

void TryLoadLightmap( Level* level ) {
	NFile lightmapTemp;
	char lightmapPath[MAX_PATH_LENGTH]{};
	CopyPathAndChangeExtension( lightmapPath, level->path, "lgt", MAX_PATH_LENGTH );
	CreateNFile( &lightmapTemp, lightmapPath, "rb" );
	
	if( !lightmapTemp.file )
		return;

	//DebugDrawAABB( Vec3( -2.97, 3.68, 0.95 ), Vec3( .25 ), 1000.0f, BLUE );
	int mapSize = NFileReadU32( &lightmapTemp );
	u8* lightMap = ( u8* ) TEMP_ALLOC( mapSize * mapSize * 4 );
	NFileRead( &lightmapTemp, lightMap, mapSize * mapSize * 4 );

	u32 lighttexid = 0;
	glGenTextures( 1, &lighttexid );
	glBindTexture( GL_TEXTURE_2D, lighttexid );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mapSize, mapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightMap );

	glActiveTexture( GL_TEXTURE0 + S2D_LIGHTMAP );
	glBindTexture( GL_TEXTURE_2D, lighttexid );
	glActiveTexture( GL_TEXTURE0 );
	NFileClose( &lightmapTemp );
}

void RenderLoadLevel( Level* level, NFile* file ) {
	TEMP_ARENA_SET;

	LevelRenderInfo* li = &renderer.levelInfo;
	li->numVertices = NFileReadU32( file );
	li->numIndices = NFileReadU32( file );
	li->numFaces = NFileReadU32( file );
	li->numBrushes = NFileReadU32( file );
	li->numTextures = NFileReadU32( file );

	TryLoadLightmap( level );
	//Load GPU Data

	u32 vertexSize = li->numVertices * sizeof( StaticVertex );
	u32 indexSize = li->numIndices * sizeof( u32 );

	StaticVertex* verticesTemp = ( StaticVertex* )TEMP_ALLOC( vertexSize );
	li->indices = ( u32* )ScratchArenaAllocate( &level->arena, indexSize );

	NFileRead( file, verticesTemp, vertexSize );
	NFileRead( file, li->indices, indexSize );

	CreateGLBuffer( &li->buffer, li->numVertices, li->numIndices, vertexSize, verticesTemp,
		indexSize, li->indices, true, false );
	GLBufferAddDefaultAttribsStatic( &li->buffer );

	//Load in CPU Data
	u32 brushSize = li->numBrushes * sizeof( RenderBrush );
	u32 faceSize = li->numFaces * sizeof( RenderBrushFace );

	//Note: Since these are allocated next to each other, it's okay to blast  li->faces with the size of both
	li->faces = ( RenderBrushFace* )ScratchArenaAllocate( &level->arena, faceSize );
	li->brushes = ( RenderBrush* )ScratchArenaAllocate( &level->arena, brushSize );
	NFileRead( file, li->faces, faceSize + brushSize );

	char* textureNamesTemp = ( char* )TEMP_ALLOC( li->numTextures * NAME_BUF_LEN );
	NFileRead( file, textureNamesTemp, li->numTextures * NAME_BUF_LEN );

	Texture** loadedTextures = ( Texture** )TEMP_ALLOC( li->numTextures * sizeof( Texture* ) );

	// Load in textures
	for (int i = 0; i < li->numTextures; i++) {
		char* name = &textureNamesTemp[NAME_BUF_LEN * i];

		char fullName[64]{};
		strcpy( fullName, "res/textures/" );
		strcat( fullName, name );
		strcat( fullName, ".png" );

		loadedTextures[i] = TextureManagerLoadTextureFromFile( fullName );
	}

	u32* numTrianglesPerTexture = ( u32* )TEMP_ALLOC( li->numTextures * sizeof( u32 ) );
	NFileRead( file, numTrianglesPerTexture, li->numTextures * sizeof( u32 ) );

	li->textureChains = ( TextureChain* )ScratchArenaAllocate( &level->arena, li->numTextures * sizeof( TextureChain ) );

	for (int i = 0; i < li->numTextures; i++) {
		TextureChain* chain = &li->textureChains[i];
		chain->texture = loadedTextures[i];
		chain->firstIndexOfTriangles = ( u32* )ScratchArenaAllocate( &level->arena, numTrianglesPerTexture[i] * sizeof( u32 ) );
		chain->numTriangles = 0;
	}

}

void RenderUnloadLevel() {
	//TODO unload textures, could maybe point to new level information to see if keep or not?
	memset( &renderer.levelInfo, 0, sizeof( renderer.levelInfo ) );
	renderer.lightHead = 0;
	PoolArenaFreeAll( &renderer.lightArena );
}

void RenderDrawHealthBar( Vec2 pos, Vec2 size, int hp, int maxHp ) {
	//Red Back
	//Green Front

	float healthPercent = ( float )hp / ( float )maxHp;
	Vec2 greenBarSize( size.x * healthPercent, size.y );

	if ( greenBarSize.x > 0 )
		RenderDrawQuadColored( pos, greenBarSize, Vec3( 0, 1, 0 ) );

	RenderDrawQuadColored( pos, size, Vec3( 1, 0, 0 ) );
}

void RenderLoadFontFromFile() {
	TEMP_ARENA_SET
		NFile file{};
	CreateNFile( &file, "res/misc/font.txt", "rb" );
	u32 length = file.length;

	char* buffer = ( char* )TEMP_ALLOC( length );
	NFileRead( &file, buffer, length );

	Parser parser( buffer, length );
	parser.ReadToken();

	parser.SkipUntilTokenWithString( "size" );
	parser.ReadToken();
	renderer.font.glyphSize = parser.ParseIntEqualInFront();

	parser.SkipUntilTokenWithString( "lineHeight" );
	parser.ReadToken();
	renderer.font.lineHeight = parser.ParseIntEqualInFront();

	parser.ExpectedTokenString( "base" );
	renderer.font.base = parser.ParseIntEqualInFront();

	parser.ExpectedTokenString( "scaleW" );
	renderer.font.imageSizeX = parser.ParseIntEqualInFront();

	parser.ExpectedTokenString( "scaleH" );
	renderer.font.imageSizeY = parser.ParseIntEqualInFront();

	parser.SkipUntilTokenWithString( "count" );
	parser.ReadToken();
	renderer.font.numGlyphs = parser.ParseIntEqualInFront();

	renderer.font.glyphs = ( BitmapGlyph* )ScratchArenaAllocate( &globalArena, renderer.font.numGlyphs * sizeof( BitmapGlyph ) );
	for (int i = 0; i < renderer.font.numGlyphs; i++) {
		BitmapGlyph* glyph = &renderer.font.glyphs[i];

		parser.ExpectedTokenString( "char" );

		parser.ExpectedTokenString( "id" );
		glyph->ascii = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "x" );
		glyph->x = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "y" );
		glyph->y = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "width" );
		glyph->width = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "height" );
		glyph->height = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "xoffset" );
		glyph->xoffset = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "yoffset" );
		glyph->yoffset = parser.ParseIntEqualInFront();

		parser.ExpectedTokenString( "xadvance" );
		glyph->xadvance = parser.ParseIntEqualInFront();

		//Double jsut to be safe. probably dont need
		double x0 = ( double )glyph->x / ( double )renderer.font.imageSizeX;
		double y0 = ( double )glyph->y / ( double )renderer.font.imageSizeY;

		double x1 = (( double )glyph->x + glyph->width) / ( double )renderer.font.imageSizeX;
		double y1 = (( double )glyph->y + glyph->height) / ( double )renderer.font.imageSizeY;

		glyph->tMin = Vec2( ( float )x0, ( float )y0 );
		glyph->tMax = Vec2( ( float )x1, ( float )y1 );
	}

}

BitmapGlyph* RenderGetGlyph( char c ) {
	for (int i = 0; i < renderer.font.numGlyphs; i++) {
		if (renderer.font.glyphs[i].ascii == c)
			return &renderer.font.glyphs[i];
	}

	LOG_WARNING( LGS_RENDERER, "Could not find glpyh %c\n" );
	return &renderer.font.glyphs[0];
}

void RenderDrawFontBatch() {
	if (renderer.numGlyphsBatched == 0)
		return;

	RenderSetShader( &renderer, renderer.shaders[SHADER_UI] );
	ShaderSetInt( &renderer, renderer.shaders[SHADER_UI], "solidColor", false );
	ShaderSetInt( &renderer, renderer.shaders[SHADER_UI], "albedo", 0 );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_UI], "model", Mat4( 1.0 ) );


	nglActiveTexture( GL_TEXTURE0 + S2D_ALBEDO );
	nglBindTexture( GL_TEXTURE_2D, renderer.fontTex->id );

	nglBindVertexArray( renderer.fontBuffer.vao );
	nglBindBuffer( GL_ARRAY_BUFFER, renderer.fontBuffer.vbo );
	nglBufferSubData( GL_ARRAY_BUFFER, 0, 4 * renderer.numGlyphsBatched * sizeof( FontVert ), renderer.glyphs );
	nglDrawElements( GL_TRIANGLES, renderer.numGlyphsBatched * 6, GL_UNSIGNED_INT, 0 );
	renderer.numGlyphsBatched = 0;
}

void RenderDrawChar( Vec2 pos, BitmapGlyph* glyph, float scale ) {
	if (renderer.numGlyphsBatched == FONT_BATCH_SIZE) {
		LOG_ERROR( LGS_RENDERER, "OUT OF ROOM FONT BATCH\N" );
		return;
	}

	FontVert* verts = &renderer.glyphs[4 * renderer.numGlyphsBatched++];

	//(hack)
	if (glyph->ascii == '.')
		pos.y += (scale / 2.5);

	verts[0].pos = Vec2( scale, scale ) + pos;// top right
	verts[1].pos = Vec2( scale, 0.0 ) + pos;// bottom right
	verts[2].pos = Vec2( 0.0, 0.0 ) + pos;// bottom left
	verts[3].pos = Vec2( 0.0, scale ) + pos;// top left 

	verts[0].tex = Vec2( glyph->tMax.x, glyph->tMax.y );
	verts[1].tex = Vec2( glyph->tMax.x, glyph->tMin.y );
	verts[2].tex = Vec2( glyph->tMin.x, glyph->tMin.y );
	verts[3].tex = Vec2( glyph->tMin.x, glyph->tMax.y );
}

void RenderDrawText( Vec2 pos, float fontSize, const char* string ) {
	Vec2 startPos = pos;

	int len = strlen( string );

	float scale = fontSize / renderer.font.glyphSize;

	for (int i = 0; i < len; i++) {
		if (string[i] == '\t')
			continue;

		if (string[i] == ' ') {
			pos.x += renderer.font.glyphSize * scale;
			continue;
		}

		if (string[i] == '\n') {
			pos.y += renderer.font.lineHeight * scale;
			pos.x = startPos.x;
			continue;
		}

		BitmapGlyph* glyph = RenderGetGlyph( string[i] );
		RenderDrawChar( pos, glyph, fontSize );

		pos.x += (glyph->xadvance) * scale;
	}
}

void nglDrawArrays( GLenum mode, GLint first, GLsizei count ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->drawArrayCalls++;
	info->drawArrayTriangleCount += count;

	glDrawArrays( mode, first, count );
}
void nglDrawElements( GLenum mode, GLsizei count, GLenum type, const void* indices ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->drawElementCalls++;
	info->drawElementTriangleCount += count;

	glDrawElements( mode, count, type, indices );
}
void nglBindVertexArray( GLuint array ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->bindVaoCalls++;

	glBindVertexArray( array );
}
void nglBindTexture( GLenum target, GLuint texture ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->bindTextureCalls++;

	glBindTexture( target, texture );
}
void nglBindBuffer( GLenum target, GLuint buffer ) {
	glBindBuffer( target, buffer );
}
void nglBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->bufferSubDataCalls++;
	info->bufferSubdataBytes += size;

	glBufferSubData( target, offset, size, data );
}

void nglActiveTexture( GLenum texture ) {
	FrameInfo* info = &renderer.frameInfos[renderer.currentFrameInfo];
	info->activeTextureCalls++;
	glActiveTexture( texture );
}

int sortRM( const void* a, const void* b ) {
	RenderModel* ra = *( RenderModel** ) a;
	RenderModel* rb = *( RenderModel** ) b;
	
	if( ra->model > rb->model )
		return -1;
	if( rb->model > ra->model )
		return 1;

	return 0;
}

void RenderDrawAllEntities() {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		StoredEntity* stored = &entityManager.entities[i];

		if (stored->state != ACTIVE_ACTIVE)
			continue;

		if (stored->entity.renderModel != 0)
			RenderDrawEntity( &stored->entity );
	}
}

void RenderDrawAllProjectiles() {
	for (int i = 0; i < entityManager.numProjectiles; i++) {
		Projectile* projectile = &entityManager.projectiles[i];
		Mat4 t = glm::translate( Mat4( 1.0 ), projectile->collider.offset + projectile->collider.bounds.center );
		Mat4 s = glm::scale( Mat4( 1.0 ), projectile->model.scale );
		Mat4 r = glm::toMat4(projectile->model.rotation);
		Mat4 trs = t * r * s;
		RenderDrawModel( &renderer, projectile->model.model, trs );
	}
}

void RenderDrawGun() {
	Player* player = entityManager.player;
	if (!player || !player->currentWeapon || !player->currentWeapon->renderModel->model)
		return;

	glClear( GL_DEPTH_BUFFER_BIT );
	Mat4 t = glm::translate( Mat4( 1.0 ), player->currentWeapon->pos );
	Mat4 r = glm::toMat4( player->currentWeapon->rotation );
	Mat4 s = glm::scale( Mat4( 1.0 ), player->currentWeapon->renderModel->scale );
	Mat4 model = t * r * s;
	model = glm::inverse( renderer.camera.GetViewMatrix() ) * model;
	RenderDrawModel( &renderer, entityManager.player->currentWeapon->renderModel->model, model, entityManager.player->currentWeapon->renderModel->pose );
}

void RenderDrawAllPickups() {
	for (int i = 0; i < entityManager.numPickups; i++) {
		Pickup* pickup = &entityManager.pickups[i];
		RenderModel* rm = &pickup->renderModel;
		if( !rm->model )
			continue;

		Mat4 t = glm::translate( Mat4( 1.0 ), rm->translation + pickup->bounds.center );
		Mat4 r = glm::toMat4( rm->rotation );
		Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( rm->scale ) );
		Mat4 trs = t * r * s;

		Shader* shader = 0;//Shader override

		glEnable( GL_BLEND );
		switch( pickup->flags ) {
			case PICKUP_PLACE_KEY_BLUE:
			case PICKUP_PLACE_KEY_RED:
				shader = renderer.shaders[SHADER_PLACE];
		}
		RenderDrawModel( &renderer, rm->model, trs, 0, shader );
		glDisable( GL_BLEND );
	}
}

void RenderDrawAllRigidBodies() {
	for (int i = 0; i < MAX_RIGIDBODIES; i++) {
		RigidBody* body = &physics.rigidBodies[i];
		if (body->state == RB_NONE)
			continue;

		//DebugDrawSphere( body->pos, body->radius );
		if (body->model) {
			Mat4 t = glm::translate( Mat4( 1.0 ), body->pos + body->visualOffset );
			Mat4 r = glm::toMat4( body->rotOffset );
			Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( body->modelScale ) );
			Mat4 trs = t * r * s;
			RenderDrawModel( &renderer, body->model, trs, body->pose );
		}
	}
}

//This does not handle actually removing it. The UpdateAndDrawEmitter function does
void RemoveEmitter( ParticleEmitter2* emitter ) {
	emitter->maxEmitterLifeTime = 0;
	renderer.emittersDirty = true;
}

ParticleEmitter2* NewParticleEmitter() {
	if (renderer.numEmitters == MAX_PARTICLE_EMITTERS) {
		LOG_WARNING ( LGS_RENDERER, "OUT OF PARTICLE EMITTERS" );
		return &renderer.emitters.emitters[0];
	}

	ParticleEmitter2* emitter = renderer.emitters.freeList[0];

	renderer.emitters.activeList[renderer.numEmitters] = emitter;
	renderer.emitters.freeList[0] = renderer.emitters.freeList[MAX_PARTICLE_EMITTERS - renderer.numEmitters - 1];
	renderer.numEmitters++;

	memset ( emitter, 0, sizeof ( *emitter ) );
	emitter->scale = Vec2 ( 1 );
	return emitter;
}

//Complete Hack
void RenderDrawMuzzleFlash( Texture* texture ) {
	Weapon* weapon = entityManager.player->currentWeapon;

	if (weapon->muzzleFlashTime <= gameTime)
		return;


	Skeleton* skel = weapon->renderModel->model->skeleton;
	Node* node = 0;
	for (int i = 0; i < skel->numNodes; i++) {
		if (!strcmp( skel->joints[i].name, "MuzzleFlash" )) {
			node = &skel->joints[i];
		}
	}

	if (!node)
		return;

	//	Vec3 rpos = entityManager.player->pos + Vec3()
	//		+ revolver->pos;
	Player* player = entityManager.player;
	Mat4 t = glm::translate( Mat4( 1.0 ), weapon->pos + Vec3( weapon->renderModel->pose->pose[node->index].t - Vec3( .2f, 0.35, 0 ) ) );
	Mat4 r = glm::toMat4( player->revolver.rotation );
	Mat4 s = glm::scale( Mat4( 1.0 ), player->revolver.renderModel->scale );
	Mat4 model2 = t * r * s;
	model2 = glm::inverse( renderer.camera.GetViewMatrix() ) * model2;


	//Vec3 pos = renderer.camera.Position + renderer.camera.Front;
	Vec3 pos = model2[3];
	Vec3 playerPos = renderer.camera.Position;

	Vec3 dirFromPlayer = pos - playerPos;
	float theta = glm::atan( dirFromPlayer.y, dirFromPlayer.x );
	float dist2D = glm::sqrt( dirFromPlayer.x * dirFromPlayer.x
		+ dirFromPlayer.y * dirFromPlayer.y );
	float phi = glm::atan( -dirFromPlayer.z, dist2D );

	Mat4 model = glm::translate( Mat4( 1.0 ), pos );
	model =
		model * glm::eulerAngleXYZ( 0.0f, 0.0f, theta ) *
		glm::eulerAngleXYZ( 0.0f, phi, 0.0f );
	model = glm::scale( model, Vec3( .3f ) );


	Shader* shader = renderer.shaders[SHADER_BILLBOARD];
	RenderSetShader( &renderer, shader );
	ShaderSetMat4( &renderer, shader, "model", model );
	ShaderSetInt( &renderer, shader, "albedo", S2D_ALBEDO );

	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glActiveTexture( GL_TEXTURE0 + S2D_ALBEDO );
	glBindTexture( GL_TEXTURE_2D, renderer.muzzleFlash->id );

	nglBindVertexArray( renderer.billboard.vao );
	glDrawArrays( GL_TRIANGLES, 0, 6 );

	glDisable ( GL_BLEND );

}

void RenderDrawConsole() {
	if( !console.IsOpen() )
		return;

	glDisable( GL_DEPTH_TEST );

	float currentOpenY = console.openMax * console.openT * 720.0f;
	RenderDrawQuadColored( Vec2( 0 ), Vec2( 1280.0f, currentOpenY ), console.color);
	//Draw the bar you can type in

	float fontScale = ( float ) console.fontSize / ( float ) renderer.font.glyphSize;
	float barHeight = fontScale * renderer.font.lineHeight;

	RenderDrawQuadColored( Vec2( 0, currentOpenY - barHeight ), Vec2( 1280.0f, barHeight ), console.color * .6f);
	currentOpenY - barHeight;

	//  ==============
	//	Console Text
	//  =============
	float scale = console.fontSize / renderer.font.glyphSize;
	float xOffset = console.lineOffsetStartX * scale;
	float currentY = currentOpenY - barHeight / 2;
	ConsoleCommand* currentCommand = &console.commandHistory[console.currentViewingCommandIndex];

	RenderDrawText( Vec2( xOffset, currentY ), console.fontSize, currentCommand->data );
	//Little Blippy thing
	if( console.blipTime < console.blipLength ) {
		//int blipX = ( FindPixelOffsetOfString( &font, currentCommand->data, console.cursorLoc ) + 10 ) * scale;
		//AddSingleCharToBatch( Vec3( blipX, currentY, 0 ), console.fontSize, '|', true );
	}

	currentY -= renderer.font.lineHeight * scale;

	for( int i = 1; currentY >= 0; i++, currentY -= renderer.font.lineHeight * scale ) {
		RenderDrawText( Vec2( xOffset, currentY ), console.fontSize, console.GetHistoryRelative( -i + console.scrollAmount )->data );
	}

	glEnable( GL_DEPTH_TEST );
}

Light* NewLight() {
	LightNode* light = ( LightNode* ) PoolArenaAllocate( &renderer.lightArena );

	if( !light ) {
		LOG_WARNING( LGS_RENDERER, "CAN NOT ALLOCATE NEW LIGHT\n" );
		return 0;
	}

	if( renderer.lightHead )
		renderer.lightHead->prev = light;

	light->next = renderer.lightHead;
	light->prev = 0;
	renderer.lightHead = light;

	memset( &light->light, 0, sizeof( Light ) );
	LightSetAttenuation( &light->light, 30 );
	light->light.color = Vec3( 1 );
	light->light.intensity = 1.0f;

	return &light->light;
}

void RemoveLight( Light* light ) {
	LightNode* node = ( LightNode* ) light;
	if( !node->prev ) {
		renderer.lightHead = node->next;
		if( node->next )
			node->next->prev = 0;
	}
	else {
		node->prev->next = node->next;
		if( node->next )
			node->next->prev = node->prev;
	}
}

void RenderUploadStaticLights() {
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, renderer.worldViewSSBO );
	//Upload Lights
	int worldViewOffset = offsetof( WorldView, staticLights );
	glBufferSubData( GL_SHADER_STORAGE_BUFFER, worldViewOffset, sizeof(Light) * renderer.worldView.numStaticLights, &renderer.worldView.staticLights);
	//Upload lightCount
	//worldViewOffset = offsetof( WorldView, numStaticLights );
	//glBufferSubData( GL_SHADER_STORAGE_BUFFER, worldViewOffset, 4, &renderer.worldView );
	//glBufferSubData( GL_SHADER_STORAGE_BUFFER, 0, sizeof( WorldView ), &renderer.worldView );

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
}