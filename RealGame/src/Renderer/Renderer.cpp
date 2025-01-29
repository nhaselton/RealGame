#include "Renderer.h"
#include "Resources/ShaderManager.h"
#include "Resources/Shader.h"
#include "Resources\ModelManager.h"
#include "Renderer\DebugRenderer.h"
#include "Resources\TextureManager.h"
#include "resources/Level.h"
#include "Game/Entity.h"

extern ModelManager modelManager;

//Only adds view projection model
inline void ShaderBuiltInsInit( Renderer* renderer ) {
	for ( int i = 0; i < SHADER_LAST; i++ ) {
		Shader* shader = renderer->shaders[i];
		if ( !shader->updateMVP )
			continue;
		RenderSetShader( renderer, shader );
		ShaderAddArg( &shaderManager, shader, SHADER_ARG_MAT4, "view" );
		ShaderSetMat4( renderer, shader, "view", Mat4( 1.0 ) );

		ShaderAddArg( &shaderManager, shader, SHADER_ARG_MAT4, "model" );
		ShaderSetMat4( renderer, shader, "model", Mat4( 1.0 ) );

		ShaderAddArg( &shaderManager, shader, SHADER_ARG_MAT4, "projection" );
		ShaderSetMat4( renderer, shader, "projection", renderer->projection );
	}
}

inline void ShaderBuiltInsSetPVM( Renderer* renderer, Mat4 p, Mat4 v, Mat4 m ) {
	for ( int i = 0; i < SHADER_LAST; i++ ) {
		Shader* shader = renderer->shaders[i];
		if ( !shader->updateMVP )
			continue;
		RenderSetShader( renderer, shader );
		ShaderSetMat4( renderer, shader, "view", v );
		ShaderSetMat4( renderer, shader, "projection", p );
		ShaderSetMat4( renderer, shader, "model", m );
	}
}

void CreateShaders( Renderer* renderer ) {
	//Brute force here. Set nons at end
	//XYZRGB
	renderer->shaders[SHADER_XYZRGB] = ShaderManagerCreateShader( &shaderManager, "res/shaders/xyzrgb/xyzrgb.vert", "res/shaders/xyzrgb/xyzrgb.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_XYZRGB] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_XYZRGB], SHADER_ARG_VEC3, "color" );

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
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD], "albedo", S2D_ALBEDO );

	//Standrard Skinned
	renderer->shaders[SHADER_STANDARD_SKINNED] = ShaderManagerCreateShader( &shaderManager, "res/shaders/standardskinned/standardskinned.vert", "res/shaders/standardskinned/standardskinned.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD_SKINNED] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_INT, "albedo" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_STANDARD_SKINNED], "albedo", S2D_ALBEDO );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_STANDARD_SKINNED], SHADER_ARG_MAT4_ARRAY, "bones" );
	ShaderSetMat4Array( renderer, renderer->shaders[SHADER_STANDARD_SKINNED], "bones", renderer->mat4Array, 100 );

	//Line
	renderer->shaders[SHADER_LINE_SHADER] = ShaderManagerCreateShader( &shaderManager, "res/shaders/line/line.vert", "res/shaders/line/line.frag" );

	//UI
	renderer->shaders[SHADER_UI] = ShaderManagerCreateShader( &shaderManager, "res/shaders/ui/ui.vert", "res/shaders/ui/ui.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_UI] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_INT, "albedo" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_INT, "solidColor" );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_UI], SHADER_ARG_VEC3, "color" );
	
	ShaderSetInt( renderer, renderer->shaders[SHADER_UI], "albedo", S2D_ALBEDO );
	ShaderSetInt( renderer, renderer->shaders[SHADER_UI], "solidColor", true );
	ShaderSetVec3( renderer, renderer->shaders[SHADER_UI], "color", Vec3( 1, 0, 0 ) );

	renderer->shaders[SHADER_SKYBOX] = ShaderManagerCreateShader( &shaderManager, "res/shaders/skybox/skybox.vert", "res/shaders/skybox/skybox.frag" );
	RenderSetShader( renderer, renderer->shaders[SHADER_SKYBOX] );
	ShaderAddArg( &shaderManager, renderer->shaders[SHADER_SKYBOX], SHADER_ARG_INT, "albedo" );
	ShaderSetInt( renderer, renderer->shaders[SHADER_SKYBOX], "albedo", S3D_SKYBOX );

	for ( int i = 0; i < SHADER_LAST; i++ )
		renderer->shaders[i]->updateMVP = true;

	//Note: Must do this for all non UPDATE-MVP 
	Shader* ui = renderer->shaders[SHADER_UI];
	RenderSetShader( renderer, ui );
	ui->updateMVP = false;
	ShaderAddArg( &shaderManager, ui, SHADER_ARG_MAT4, "projection" );
	ShaderSetMat4( renderer, ui, "projection", renderer->orthographic );
	ShaderAddArg( &shaderManager, ui, SHADER_ARG_MAT4, "model" );
	ShaderSetMat4( renderer, ui, "model", Mat4( 1.0 ) );

}

void CreateRenderer( Renderer* renderer, void* memory, u32 size ) {
	Mat4 view( 1.0 );
	renderer->projection = glm::perspective( glm::radians( 90.0f ), 16.0f / 9.0f, .1f, 2048.0f );
	renderer->orthographic = glm::ortho( 0.0f, ( float ) window.width, ( float ) window.height, 0.0f,-1.0f, 1.0f );
	Vec4 color( 1, 0, 0, 1 );

	renderer->cube = ModelManagerAllocate( &modelManager,"res/models/cube.glb" );
	renderer->sphere = ModelManagerAllocate( &modelManager, "res/models/sphere.glb" );

	float quadVertices[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
	};

	CreateGLBuffer( &renderer->quadBuffer, 4, 0, sizeof( quadVertices ), quadVertices, 0, 0, true, true );
	GLBufferAddAttribute( &renderer->quadBuffer, 0, 2, GL_FLOAT, 4 * sizeof( float ), 0 );
	GLBufferAddAttribute( &renderer->quadBuffer, 1, 2, GL_FLOAT, 4 * sizeof( float ), ( void* ) ( 2 * sizeof( float ) ) );

	CreateShaders( renderer );
	//Sets up the PVMs
	ShaderBuiltInsInit( renderer );
	
	RenderInitFont();

	renderer->crosshairTex = TextureManagerLoadTextureFromFile( "res/textures/crosshair.png" );
	renderer->healthTex = TextureManagerLoadTextureFromFile( "res/textures/health.png" );

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
		for ( int i = 0; i < 6; i++ ) {
			Texture* texture = &renderer->skybox.faces[i];
			u8* data = ( u8* ) stbi_load( paths[i], &texture->width, &texture->height, &texture->channels, 0 );
			if ( !data ) {
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
		GLBufferAddAttribute( &renderer->skybox.buffer, 0, 3, GL_FLOAT, 3 * sizeof( float ), ( void* ) 0 );
	}

}

void RenderInitFont() {
	renderer.fontTex = TextureManagerLoadTextureFromFile( "res/textures/font.png" );
	RenderLoadFontFromFile();

	TEMP_ARENA_SET;
	u32* indices = ( u32* ) TEMP_ALLOC( FONT_BATCH_SIZE * sizeof( indices[0] ) * 6 );
	int offset = 0;
	for ( int i = 0; i < FONT_BATCH_SIZE * 4; i += 6 ) {
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
	GLBufferAddAttribute( &renderer.fontBuffer, 1, 2, GL_FLOAT, 4 * sizeof( float ), (void*) (2 * sizeof(float)) ); //tex
}

void RenderStartFrame( Renderer* renderer ){
	glClearColor( 0, .25, .45, 1 );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
}

struct TextureList {
	Texture* texture;
	u32 triCount;
	u32* firstIndexOfTriangles; //Can add ++ and ++++ after
};

void RenderDrawLevel( Renderer* renderer ) {
	TEMP_ARENA_SET;

	TextureChain* textureChains = renderer->levelInfo.textureChains;
	u32 totalIndices = 0;

	//Frist Loop through and get counts of each surface
	for ( u32 i = 0; i < renderer->levelInfo.numFaces; i++ ) {
		RenderBrushFace* face = &renderer->levelInfo.faces[i];
		TextureChain* chain = &textureChains[face->textureIndex];

		totalIndices += face->numIndices;
		//Add each triangle
		for ( int n = 0; n < face->numIndices; n += 3 )
			chain->firstIndexOfTriangles[chain->numTriangles++] = face->firstIndex + n;
	}

	//Now Loop through again and construct the index ptr[]
	u32* indices = ( u32* ) TEMP_ALLOC( totalIndices * sizeof( u32 ) );
	u32 offset = 0;
	for ( int i = 0; i < renderer->levelInfo.numTextures; i++ ) {
		TextureChain* chain = &textureChains[i];

		for ( int n = 0; n < chain->numTriangles; n++ ) {
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 0];
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 1];
			indices[offset++] = renderer->levelInfo.indices[chain->firstIndexOfTriangles[n] + 2];
		}
	}

	//Upload indices to GPU
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD] );
	glBindVertexArray( renderer->levelInfo.buffer.vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->levelInfo.buffer.ebo );
	glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, totalIndices * sizeof( u32 ), indices );

	offset = 0;
	glActiveTexture( GL_TEXTURE0 );
	for ( int i = 0; i < renderer->levelInfo.numTextures; i++ ) {
		TextureChain* chain = &textureChains[i];

		if ( chain->numTriangles == 0 )
			continue;

		if ( !chain->texture )
			glBindTexture( GL_TEXTURE_2D, 0 );
		else
			glBindTexture( GL_TEXTURE_2D, chain->texture->id );

		//Go back through buffer and do 1 draw call per surface
		glDrawElements( GL_TRIANGLES, chain->numTriangles * 3, GL_UNSIGNED_INT, ( void* ) ( offset * sizeof( u32 ) ));
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
	
	glBindVertexArray( renderer.quadBuffer.vao );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void RenderDrawQuadTextured( Vec2 pos, Vec2 size, Texture* texture ) {
	Shader* shader = renderer.shaders[SHADER_UI];
	RenderSetShader( &renderer, shader );

	Mat4 projection = glm::ortho( 0.0f, 1280.0f, 720.0f, 0.0f, -1.0f, 1.0f );
	Mat4 t = glm::translate( Mat4( 1.0 ), Vec3( pos, 0 ) );
	Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( size, 1.0f ) );
	Mat4 trs = t * s;

	ShaderSetMat4( &renderer, shader, "model", trs );
	ShaderSetInt( &renderer, shader, "solidColor", false );

	if ( texture ) {
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texture->id );
	}

	glBindVertexArray( renderer.quadBuffer.vao );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void RenderDrawFrame( Renderer* renderer, float dt ){
	//reset texture chains
	for ( int i = 0; i < renderer->levelInfo.numTextures; i++ ) {
		renderer->levelInfo.textureChains[i].numTriangles = 0;
	}

	ShaderBuiltInsSetPVM( renderer, renderer->projection, renderer->camera.GetViewMatrix(), Mat4( 1.0 ) );

	RenderDrawLevel( renderer );
	DebugRendererFrame( renderer->camera.GetViewMatrix(), renderer->projection, dt );	

	RenderDrawText( Vec2( 0,300 ), 32.0f, "The Quick Brown Fox Jumped Over The Lazy\nSleeping Dog" );
	RenderDrawFontBatch();

	glDepthFunc( GL_LEQUAL );
	glBindVertexArray( renderer->skybox.buffer.vao );
	RenderSetShader( renderer, renderer->shaders[SHADER_SKYBOX] );
	ShaderSetMat4( renderer, renderer->shaders[SHADER_SKYBOX], "view", Mat4( Mat3( renderer->camera.GetViewMatrix() ) ) );
	ShaderSetMat4( renderer, renderer->shaders[SHADER_SKYBOX], "model", Mat4( 1.0f ) );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	glDepthFunc( GL_LESS );

}

void RenderEndFrame( Renderer* renderer ) {

}

void BuildSkeleton( Skeleton* skeleton, Node* root, Mat4 parent, Mat4* matrices ) {
	Mat4 t = glm::translate( Mat4( 1.0 ), root->t );
	Mat4 r = glm::toMat4( root->r );
	Mat4 s = glm::scale( Mat4( 1.0 ), root ->s );
	
	Mat4 local = t * r * s;
	Mat4 global = parent * local;
	
	if ( root->boneID != -1 )
		matrices[root->boneID] = global * skeleton->inverseBinds[root->boneID] ;
	
	for ( int i = 0; i < root->numChildren; i++ )
		BuildSkeleton( skeleton, root->children[i], global, matrices );

}

void RenderSetShader( Renderer* renderer, Shader* newShader ) {
	if ( !newShader || newShader->id == -1 ) {
		LOG_ASSERT( LGS_RENDERER, "Trying to use shader that does not exist" );
		return;
	}
	//if ( newShader->id == renderer->currentShaderID ) {
	//	LOG_WARNING( LGS_RENDERER, "Setting active shader to same shader\n" );
	//}

	glUseProgram( newShader->id );
	renderer->currentShaderID = newShader->id;
}

void RenderDrawModel( Renderer* renderer, Model* model, Mat4 offset, SkeletonPose* pose ) {
	Shader* shader = ( model->skeleton == 0 ) ? renderer->shaders[SHADER_STANDARD] : renderer->shaders[SHADER_STANDARD_SKINNED];
	RenderSetShader( renderer, shader );

	//Setup mat4 array
	if ( model->skeleton && pose ) {
		BuildSkeleton( model->skeleton, &model->skeleton->joints[model->skeleton->root], Mat4(1.0f), renderer->mat4Array);
		ShaderSetMat4Array( renderer, shader, "bones", pose->globalPose, model->skeleton->numBones );
	}	

	ShaderSetMat4( renderer, shader, "model", offset );

	for ( u32 i = 0; i < model->numMeshes; i++ ) {
		if ( model->meshes[i].texture != 0 ) {
			glActiveTexture( GL_TEXTURE0 + S2D_ALBEDO );
			glBindTexture( GL_TEXTURE_2D, model->meshes[i].texture->id );
		}

		glBindVertexArray( model->meshes[i].buffer.vao );
		glDrawElements( GL_TRIANGLES, model->meshes[i].numIndices, GL_UNSIGNED_INT, (void*) 0 );

	}
}

void RenderDrawEntity( Entity* entity ) {
	Mat4 t = glm::translate( Mat4( 1.0 ), entity->renderModel->translation + entity->pos );
	Mat4 rmr = glm::toMat4( entity->renderModel->rotation );
	Mat4 entr = glm::toMat4( entity->rotation );
	Mat4 r = entr * rmr;
	Mat4 s = glm::scale( Mat4( 1.0 ), entity->renderModel->scale );
	Mat4 trs = t * r * s;

	SkeletonPose* pose = ( entity->renderModel->pose != 0 ) ? entity->renderModel->pose : 0;
	RenderDrawModel( &renderer, entity->renderModel->model, trs, pose );
}


void RenderLoadLevel( Level* level, NFile* file ) {
	TEMP_ARENA_SET;
	LevelRenderInfo* li = &renderer.levelInfo;
	li->numVertices = NFileReadU32( file );
	li->numIndices = NFileReadU32( file );
	li->numFaces = NFileReadU32( file );
	li->numBrushes = NFileReadU32( file );
	li-> numTextures = NFileReadU32( file );

	//Load GPU Data

	u32 vertexSize = li->numVertices * sizeof( DrawVertex );
	u32 indexSize = li->numIndices * sizeof( u32 );

	DrawVertex* verticesTemp = ( DrawVertex* ) TEMP_ALLOC(  vertexSize );
	li->indices = ( u32* ) ScratchArenaAllocate( &level->arena, indexSize );
	
	NFileRead( file, verticesTemp, vertexSize );
	NFileRead( file, li->indices, indexSize );

	Vec3 v0 = verticesTemp[0].pos;
	for ( int i = 0; i < 24; i++ )
		if ( glm::length( verticesTemp[i].pos - v0 ) < .001f )
			printf( "%d\n", i );
			

	CreateGLBuffer( &li->buffer, li->numVertices, li->numIndices, vertexSize, verticesTemp,
		indexSize, li->indices, true, false );
	GLBufferAddDefaultAttribs( &li->buffer );


	//Load in CPU Data
	u32 brushSize = li->numBrushes * sizeof( RenderBrush );
	u32 faceSize = li->numFaces * sizeof( RenderBrushFace );
	
	//Note: Since these are allocated next to each other, it's okay to blast  li->faces with the size of both
	li->faces = ( RenderBrushFace* ) ScratchArenaAllocate( &level->arena, faceSize );
	li->brushes = ( RenderBrush* ) ScratchArenaAllocate( &level->arena, brushSize );
	NFileRead( file, li->faces, faceSize + brushSize);

	char* textureNamesTemp = ( char* ) TEMP_ALLOC( li->numTextures * NAME_BUF_LEN );
	NFileRead( file, textureNamesTemp, li->numTextures * NAME_BUF_LEN );

	Texture** loadedTextures = (Texture**) TEMP_ALLOC( li->numTextures * sizeof( Texture* ) );

	// Load in textures
	for ( int i = 0; i < li->numTextures; i++ ) {
		char* name = &textureNamesTemp[ NAME_BUF_LEN * i];

		char fullName[64]{};
		strcpy( fullName, "res/textures/" );
		strcat( fullName, name );
		strcat( fullName, ".png" );

		loadedTextures[i] = TextureManagerLoadTextureFromFile( fullName );
	}

	//Assign faces their textures
	for ( int i = 0; i < li->numFaces; i++ ) {
		//Index is stored in pointer loc
		u32 index = (u32) li->faces[i].texture;
		li->faces[i].texture = loadedTextures[index];
	}

	u32* numTrianglesPerTexture = (u32*) TEMP_ALLOC( li->numTextures * sizeof( u32 ) );
	NFileRead( file, numTrianglesPerTexture, li->numTextures * sizeof( u32 ) );

	li->textureChains = ( TextureChain* ) ScratchArenaAllocate( &level->arena, li->numTextures * sizeof( TextureChain ) );

	for ( int i = 0; i < li->numTextures; i++ ) {
		TextureChain* chain = &li->textureChains[i];
		chain->texture = loadedTextures[i];
		chain->firstIndexOfTriangles = ( u32* ) ScratchArenaAllocate( &level->arena, numTrianglesPerTexture[i] * sizeof( u32 ) );
		chain->numTriangles = 0;
	}
}

void RenderDrawHealthBar( Vec2 pos, Vec2 size, int hp, int maxHp ) {
	//Red Back
	//Green Front

	float healthPercent = ( float ) hp / ( float ) maxHp;
	Vec2 greenBarSize( size.x * healthPercent, size.y );

	RenderDrawQuadColored( pos, greenBarSize, Vec3( 0, 1, 0 ) );
	RenderDrawQuadColored( pos, size, Vec3( 1, 0, 0 ) );
}

void RenderLoadFontFromFile() {
	TEMP_ARENA_SET
	NFile file{};
	CreateNFile( &file, "res/misc/font.txt", "rb" );
	u32 length = file.length;

	char* buffer = ( char* ) TEMP_ALLOC( length );
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

	renderer.font.glyphs = ( BitmapGlyph* ) ScratchArenaAllocate( &globalArena, renderer.font.numGlyphs * sizeof( BitmapGlyph ) );
	for ( int i = 0; i < renderer.font.numGlyphs; i++ ) {
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
		double x0 = ( double ) glyph->x / ( double ) renderer.font.imageSizeX;
		double y0 = ( double ) glyph->y / ( double ) renderer.font.imageSizeY;

		double x1 = ( ( double ) glyph->x + glyph->width ) / ( double ) renderer.font.imageSizeX;
		double y1 = ( ( double ) glyph->y + glyph->height ) / ( double ) renderer.font.imageSizeY;

		glyph->tMin = Vec2( ( float ) x0, ( float ) y0 );
		glyph->tMax = Vec2( ( float ) x1, ( float ) y1 );
	}

}

BitmapGlyph* RenderGetGlyph( char c ) {
	for ( int i = 0; i < renderer.font.numGlyphs; i++ ) {
		if ( renderer.font.glyphs[i].ascii == c )
			return &renderer.font.glyphs[i];
	}
	
	LOG_WARNING( LGS_RENDERER, "Could not find glpyh %c\n" );
	return &renderer.font.glyphs[0];
}

void RenderDrawFontBatch() {
	if ( renderer.numGlyphsBatched == 0 ) 
		return;


	RenderSetShader( &renderer, renderer.shaders[SHADER_UI] );
	ShaderSetInt( &renderer, renderer.shaders[SHADER_UI], "solidColor", false );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_UI], "model", Mat4( 1.0 ) );
	
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, renderer.fontTex->id );

	glBindVertexArray( renderer.fontBuffer.vao );
	glBindBuffer( GL_ARRAY_BUFFER, renderer.fontBuffer.vbo );
	glBufferSubData( GL_ARRAY_BUFFER, 0, 4 * renderer.numGlyphsBatched * sizeof( FontVert ), renderer.glyphs );
	glDrawElements( GL_TRIANGLES, renderer.numGlyphsBatched * 6, GL_UNSIGNED_INT, 0 );
	renderer.numGlyphsBatched = 0;
}

void RenderDrawChar( Vec2 pos, BitmapGlyph* glyph, float scale ) {
	if ( renderer.numGlyphsBatched == FONT_BATCH_SIZE ) {
		LOG_ERROR( LGS_RENDERER, "OUT OF ROOM FONT BATCH\N" );
		return;
	}

	FontVert* verts = &renderer.glyphs[4 * renderer.numGlyphsBatched++];

	//(hack)
	if ( glyph->ascii == '.' )
		pos.y += ( scale / 2.5 );

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

	for ( int i = 0; i < len; i++ ) {
		if ( string[i] == ' ' ) {
			pos.x += renderer.font.glyphSize * scale;
			continue;
		}

		if ( string[i] == '\n' ) {
			pos.y += renderer.font.lineHeight * scale;
			pos.x = startPos.x;
			continue;
		}

		BitmapGlyph* glyph = RenderGetGlyph( string[i] );
		RenderDrawChar( pos, glyph, fontSize );

		pos.x += ( glyph->xadvance ) * scale;
	}
}
