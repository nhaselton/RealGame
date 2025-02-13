#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "resources/Shader.h"
#include "DebugRenderer.h"
#include "core/Window.h"
#include "Physics\Physics.h"
#include "Game/Entity.h"

struct LineBatch {
	struct DebugLine* lines;
	u32 numLines;

	u32 vao;
	u32 vbo;

	int lineWidth;
};

static DebugRenderer drenderer;
static ScratchArena debugGlobal;


//Shader debugLineShader;
static LineBatch lineBatches[NUM_LINE_BATCHES];

static void CreatePrim( DebugPrim* prim ) {
	prim->next = drenderer.prims;
	drenderer.prims = prim;
}

static void RemovePrim( DebugPrim* lookFor ) {
	//Check if prim is head
	if ( lookFor == drenderer.prims ) {
		drenderer.prims = lookFor->next;
		PoolArenaFree( &drenderer.primPool, lookFor );
		return;
	}

	//else find previous and set it's next to prim's next
	DebugPrim* prev;
	for ( prev = drenderer.prims; prev->next != lookFor; prev = prev->next );
	prev->next = lookFor->next;
	PoolArenaFree( &drenderer.primPool, lookFor );
}
static void InitDebugLines() {
	for ( int i = 0; i < NUM_LINE_BATCHES; i++ ) {

		LineBatch* batch = &lineBatches[i];

		batch->lines = ( DebugLine* ) ScratchArenaAllocate( &debugGlobal, MAX_LINES_BATCH * sizeof( DebugLine ) );
		batch->lineWidth = 0;

		glGenVertexArrays( 1, &batch->vao );
		glGenBuffers( 1, &batch->vbo );

		glBindVertexArray( batch->vao );

		glBindBuffer( GL_ARRAY_BUFFER, batch->vbo );
		glBufferData( GL_ARRAY_BUFFER, MAX_LINES_BATCH * sizeof( DebugLine ), batch->lines, GL_STATIC_DRAW );

		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, offsetof( DebugLine, b ), ( void* ) 0 );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, offsetof( DebugLine, b ), ( void* ) offsetof( DebugLine, colora ) );
		glEnableVertexAttribArray( 1 );
	}


	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glBindVertexArray( 0 );
}

#include "Resources\ModelManager.h"
void CreateDebugRenderer( Renderer* renderer, void* memory, u32 size ) {
	CreateScratchArena( &debugGlobal, size, memory, &globalArena, "Debug Renderer" );
	CreatePoolArena( &drenderer.primPool, sizeof( DebugPrim ), MAX_PRIMS, 
		ScratchArenaAllocate( &debugGlobal, sizeof( DebugPrim ) * MAX_PRIMS ),
		&debugGlobal, "Prim Allocator" );

	//debugLineShader.Create( "res/shaders/debugLine" );
	InitDebugLines();

}
void DrawLineBatch( LineBatch* batch ) {
	//lineShader->Use();
	RenderSetShader( &renderer, renderer.shaders[SHADER_LINE_SHADER] );

	//todo fix this. Maybe separate queues for depth & non depth
	glEnable( GL_DEPTH_TEST );

	if ( batch->numLines > 0 ) {
		glLineWidth( batch->lineWidth );
		glBindVertexArray( batch->vao );
		glBindBuffer( GL_ARRAY_BUFFER, batch->vbo );
		glBufferSubData( GL_ARRAY_BUFFER, 0, batch->numLines * sizeof( DebugLine ), batch->lines );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glDrawArrays( GL_LINES, 0, batch->numLines * 2 );
		batch->numLines = 0;
	}
	batch->lineWidth = -1;

	RenderSetShader( &renderer, renderer.shaders[SHADER_XYZRGB] );
}

//if any batch widths are equal use it
//If none are equal then try to find an empty one
//Try to find the closest in terms of width if previous 2 failed
LineBatch* FindBestBatch(int wantWidth) {
	int closest = INT_MAX;
	int closestIndex = -1;
	int emptyIndex = -1;

	
	for ( int i = 0; i < NUM_LINE_BATCHES; i++ ) {
		if ( lineBatches[i].lineWidth == wantWidth )
			return &lineBatches[i];

		if ( lineBatches[i].lineWidth == -1 && emptyIndex == -1) {
			emptyIndex = i;
		}

		int diffInWidth = fabs( wantWidth - lineBatches[i].lineWidth );
		if ( diffInWidth < closest ) {
			closest = diffInWidth;
			closestIndex = i;
		}
	}

	if ( emptyIndex != -1 )
		return &lineBatches[emptyIndex];

	return &lineBatches[closestIndex];

}

//if no batch supplied it will pick the best
void AddLineToBatch( DebugPrim* prim , LineBatch* batch = nullptr) {
	if ( batch == nullptr ) {
		batch = FindBestBatch(prim->lineWidth);
	}

	DebugLine* line = &batch->lines[batch->numLines++];
	line->a = prim->data.line.a;
	line->b = prim->data.line.b;
	line->colora = prim->color;
	line->colorb = prim->color;

	if ( batch->numLines == MAX_LINES_BATCH )
		DrawLineBatch( batch );


}

void DebugRendererFrame( Mat4 view, Mat4 projection, float dt ) {
	RenderSetShader( &renderer,renderer.shaders[SHADER_LINE_SHADER]);
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_LINE_SHADER], "view", view );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_LINE_SHADER], "projection", projection );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_LINE_SHADER], "model", Mat4( 1.0 ) );

	RenderSetShader( &renderer, renderer.shaders[SHADER_XYZRGB] );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_XYZRGB], "view", view );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_XYZRGB], "projection", projection );
	ShaderSetMat4( &renderer, renderer.shaders[SHADER_XYZRGB], "model", Mat4( 1.0 ) );

	DebugPrim* prim = drenderer.prims;
	while ( prim != nullptr ) {
		glDepthFunc( GL_LESS );
		glLineWidth( (float) prim->lineWidth * .1 );

		if ( BIT_CHECK( prim->flags, 1 ) )
			glEnable( GL_DEPTH_TEST );
		else
			glDisable( GL_DEPTH_TEST );

		if ( BIT_CHECK( prim->flags, 0 ) )
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		else
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		switch ( prim->type ) {
		case DP_AABB:
		{
			Mat4 t = glm::translate( Mat4( 1.0 ), prim->data.aabb.center );
			Mat4 s = glm::scale( Mat4( 1.0 ), prim->data.aabb.size );
			Mat4 trs = t * s;

			RenderSetShader( &renderer, renderer.shaders[SHADER_XYZRGB] );
			ShaderSetMat4( &renderer, renderer.shaders[SHADER_XYZRGB], "model", trs );
			ShaderSetVec3( &renderer, renderer.shaders[SHADER_XYZRGB], "color", prim->color );
			nglBindVertexArray( renderer.cube->meshes[0].buffer.vao);
			nglDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, ( void* ) 0 );

			//RenderDrawModel( &renderer, renderer.cube,trs );
		}break;

		case DP_SPHERE:
		{
			Mat4 t = glm::translate( Mat4( 1.0 ), prim->data.sphere.center );
			Mat4 s = glm::scale( Mat4( 1.0 ), Vec3( prim->data.sphere.radius ) );
			Mat4 trs = t * s;
			//todo rotations
			RenderSetShader( &renderer, renderer.shaders[SHADER_XYZRGB] );
			ShaderSetMat4( &renderer, renderer.shaders[SHADER_XYZRGB], "model", trs );
			ShaderSetVec3( &renderer, renderer.shaders[SHADER_XYZRGB], "color", prim->color );
			RenderDrawModel( &renderer, renderer.sphere,trs );
		}break;
		
		case DP_LINE:
		{
			AddLineToBatch( prim );
		}break;
		default:
		{
			LOG_WARNING( LGS_RENDERER, "unkown prim type %d\n", prim->type );
		}
		}
		prim->duration -= dt;
		if ( prim->duration < 0.0f ) {
			DebugPrim* next = prim->next;
			RemovePrim( prim);
			prim = next;
			continue;
		}
		prim = prim->next;
	}

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnable( GL_DEPTH_TEST );

	//Do this even with 0 because it resets the count
	for ( int i = 0; i < 4; i++ )
		DrawLineBatch( &lineBatches[i] );
}

void DebugDrawBoundsMinMax( struct BoundsMinMax* bounds, Vec3 color , float duration, bool wireframe , float linewidth , bool depthTest ) {
	Vec3 center = ( bounds->max + bounds->min ) / 2.0f;
	Vec3 halfWidth = ( bounds->max - bounds->min );
	DebugDrawAABB( center, halfWidth, duration, color, wireframe, linewidth, depthTest );
}

void DebugDrawAABB( Vec3 center, Vec3 bounds, float duration, Vec3 color, bool wireframe, float linewidth, bool depthTest ) {
	DebugPrim* prim = ( DebugPrim* ) PoolArenaAllocateZero( &drenderer.primPool );
	if ( !prim ) return;
	prim->type = DP_AABB;
	prim->data.aabb.center = center;
	prim->data.aabb.size = bounds;
	prim->color = color;
	prim->lineWidth = linewidth / .1f;
	if ( wireframe ) BIT_SET( prim->flags, 0 );
	if ( depthTest ) BIT_SET( prim->flags, 1 );

	prim->duration = duration;
	if ( prim == NULL ) {
		printf( "out of debug prims\n" );
		return;
	}

	CreatePrim( prim );
}

void DebugDrawSphere( Vec3 center, float radius, Vec3 color, bool wireframe, float linewidth, bool depthTest, float duration ) {
	DebugPrim* prim = ( DebugPrim* ) PoolArenaAllocateZero( &drenderer.primPool );
	if ( !prim ) return;

	prim->type = DP_SPHERE;
	prim->data.sphere.center = center;
	prim->data.sphere.radius = Vec3(radius);
	prim->color = color;
	prim->lineWidth = linewidth / .1f;
	if ( wireframe ) BIT_SET( prim->flags, 0 );
	if ( depthTest ) BIT_SET( prim->flags, 1 );
	prim->duration = duration;
	if ( prim == NULL ) {
		printf( "out of debug prims\n" );
		return;
	}

	CreatePrim( prim );
}

void DebugDrawLine( Vec3 a, Vec3 b, Vec3 color, float linewidth, bool depthTest, bool screenSpace, float duration ) {
	DebugPrim* prim = ( DebugPrim* ) PoolArenaAllocateZero( &drenderer.primPool );
	if ( prim == NULL ) {
		printf( "out of debug prims\n" );
		return;
	}

	prim->type = DP_LINE;
	prim->duration = duration;
	prim->data.line.a = a;
	prim->data.line.b = b;
	prim->color = color;
	prim->lineWidth = linewidth / .1f;
	if ( depthTest ) BIT_SET( prim->flags, 1 );
	//ScreenSpace
	prim->flags = screenSpace;
	CreatePrim( prim );
}

void DebugDrawBoundsHalfWidth( BoundsHalfWidth* bounds, Vec3 color, float duration, bool wireframe, float linewidth, bool depthTest ) {
	DebugDrawAABB( bounds->center, bounds->width, duration, color, wireframe, linewidth, depthTest );
}

void DebugDrawEllipse( Vec3 center, Vec3 radius, Vec3 color, bool wireframe, bool depthTest, bool screenSpace, float duration ) {
	DebugPrim* prim = ( DebugPrim* ) PoolArenaAllocateZero( &drenderer.primPool );
	if ( !prim ) return;

	prim->type = DP_SPHERE;
	prim->data.sphere.center = center;
	prim->data.sphere.radius = radius;
	prim->color = color;
	if ( depthTest ) BIT_SET( prim->flags, 1 );
	if ( wireframe ) BIT_SET( prim->flags, 0 );
	prim->duration = duration;
	if ( prim == NULL ) {
		printf( "out of debug prims\n" );
		return;
	}

	CreatePrim( prim );
}

void DebugDrawCharacterCollider( CharacterCollider* collider, Vec3 color , bool wireframe , bool depthTest , float duration ) {
	DebugDrawAABB( collider->offset + collider->bounds.center, collider->bounds.width , duration, color, wireframe, 1, depthTest );;
	DebugDrawEllipse( collider->offset + collider->bounds.center, collider->bounds.width, color, wireframe, depthTest, duration );;
}