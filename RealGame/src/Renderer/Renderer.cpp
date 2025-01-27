#include <glad\glad.h>
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
		RenderSetShader( renderer, shader );
		ShaderSetMat4( renderer, shader, "view", v );
		ShaderSetMat4( renderer, shader, "projection", p );
		ShaderSetMat4( renderer, shader, "model", m );
	}
}

void CreateRenderer( Renderer* renderer, void* memory, u32 size ) {
	Mat4 view( 1.0 );
	renderer->projection = glm::perspective( glm::radians( 90.0f ), 16.0f / 9.0f, .1f, 2048.0f );
	Vec4 color( 1, 0, 0, 1 );

	renderer->cube = ModelManagerAllocate( &modelManager,"res/models/cube.glb" );
	renderer->sphere = ModelManagerAllocate( &modelManager, "res/models/sphere.glb" );

	//XYZRGB
	renderer->shaders[SHADER_XYZRGB] = ShaderManagerCreateShader(&shaderManager, "res/shaders/xyzrgb/xyzrgb.vert", "res/shaders/xyzrgb/xyzrgb.frag");
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

	renderer->shaders[SHADER_LINE_SHADER] = ShaderManagerCreateShader(&shaderManager, "res/shaders/line/line.vert", "res/shaders/line/line.frag");
	
	//Sets up the PVMs
	ShaderBuiltInsInit( renderer );
	

	float vertices[] = {
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//Buffer
	glEnable( GL_DEPTH_TEST );
}

void CameraMovement( Camera* camera , float dt) {
	return;
	float mx = 0, my = 0, speed = 15 * dt;
	if ( KeyDown( KEY_LEFT ) ) mx -= 3;
	if ( KeyDown( KEY_RIGHT ) ) mx += 3;
	if ( KeyDown( KEY_DOWN ) ) my -= 1.5;
	if ( KeyDown( KEY_UP ) ) my += 1.5;

	if ( KeyDown( KEY_W ) ) camera->ProcessKeyboard( FORWARD, speed );
	if ( KeyDown( KEY_S ) ) camera->ProcessKeyboard( BACKWARD, speed );
	if ( KeyDown( KEY_A ) ) camera->ProcessKeyboard( LEFT, speed );
	if ( KeyDown( KEY_D ) ) camera->ProcessKeyboard( RIGHT, speed );

	camera->ProcessMouseMovement( mx * speed * 50, my * speed * 30, true);
}

void RenderStartFrame( Renderer* renderer ){
	glClearColor( 0, .25, .45, 1 );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
}

void RenderDrawLevel( Renderer* renderer ) {
	RenderSetShader( renderer, renderer->shaders[SHADER_STANDARD] );

	glBindVertexArray( renderer->levelInfo.buffer.vao );
	glActiveTexture( GL_TEXTURE0 );

	for ( u32 i = 0; i < renderer->levelInfo.numFaces; i++ ) {
		RenderBrushFace* face = &renderer->levelInfo.faces[i];
		if ( face->texture ) {
			glActiveTexture( GL_TEXTURE0 + S2D_ALBEDO );
			glBindTexture( GL_TEXTURE_2D, face->texture->id );
		}

		glDrawElements( GL_TRIANGLES, face->numIndices, GL_UNSIGNED_INT, ( void* ) ( face->firstIndex * sizeof( u32 ) ) );
	}

}

void RenderDrawFrame( Renderer* renderer, float dt ){
	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_CCW );
	CameraMovement( &renderer->camera,dt );
	ShaderBuiltInsSetPVM( renderer, renderer->projection, renderer->camera.GetViewMatrix(), Mat4( 1.0 ) );
	RenderDrawLevel( renderer );

	DebugRendererFrame( renderer->camera.GetViewMatrix(), renderer->projection, dt );	
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

	u32 numTextures = NFileReadU32( file );
	//Load GPU Data

	u32 vertexSize = li->numVertices * sizeof( DrawVertex );
	u32 indexSize = li->numIndices * sizeof( u32 );

	DrawVertex* verticesTemp = ( DrawVertex* ) TEMP_ALLOC(  vertexSize );
	u32* indicesTemp = ( u32* ) TEMP_ALLOC( indexSize );
	
	NFileRead( file, verticesTemp, vertexSize );
	NFileRead( file, indicesTemp, indexSize );

	Vec3 v0 = verticesTemp[0].pos;
	for ( int i = 0; i < 24; i++ )
		if ( glm::length( verticesTemp[i].pos - v0 ) < .001f )
			printf( "%d\n", i );
			

	CreateGLBuffer( &li->buffer, li->numVertices, li->numIndices, vertexSize, verticesTemp,
		indexSize, indicesTemp, true, false );
	GLBufferAddDefaultAttribs( &li->buffer );


	//Load in CPU Data
	u32 brushSize = li->numBrushes * sizeof( RenderBrush );
	u32 faceSize = li->numFaces * sizeof( RenderBrushFace );
	
	//Note: Since these are allocated next to each other, it's okay to blast  li->faces with the size of both
	li->faces = ( RenderBrushFace* ) ScratchArenaAllocate( &level->arena, faceSize );
	li->brushes = ( RenderBrush* ) ScratchArenaAllocate( &level->arena, brushSize );
	NFileRead( file, li->faces, faceSize + brushSize);

	char* textureNamesTemp = ( char* ) TEMP_ALLOC( numTextures * NAME_BUF_LEN );
	NFileRead( file, textureNamesTemp, numTextures * NAME_BUF_LEN );

	Texture** loadedTextures = (Texture**) TEMP_ALLOC( numTextures * sizeof( Texture* ) );

	// Load in textures
	for ( int i = 0; i < numTextures; i++ ) {
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

	//for ( int i = 0; i < li->numIndices; i++ )
	//	printf( "%d, ", indicesTemp[i] );

	int a = 0;
}