#include "GLBuffer.h"
#include "Resources/ModelManager.h"
#include <glad\glad.h>

void CreateGLBuffer( GLBuffer* buffer, u32 numVertices, u32 numIndices, u32 vboSize, void* vboData, u32 eboSize, void* eboData, bool staticVBO, bool staticEBO ) {
	glGenVertexArrays( 1, &buffer->vao );
	glGenBuffers( 1, &buffer->vbo );
	glGenBuffers( 1, &buffer->ebo );

	glBindVertexArray( buffer->vao );

	glBindBuffer( GL_ARRAY_BUFFER, buffer->vbo );
	glBufferData( GL_ARRAY_BUFFER, vboSize, vboData, ( staticVBO ) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );

	if ( eboSize > 0 ) {
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer->ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, eboSize, eboData, ( staticEBO ) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );
	}

	buffer->numVertices = numVertices;
	buffer->numIndices = numIndices;
	buffer->vboSize = vboSize;
	buffer->eboSize = eboSize;
}

void GLBufferAddAttributeI( GLBuffer* buffer, u32 index, u32 size, u32 type, i32 stride, const void* offset ) {
	glVertexAttribIPointer( index, size, type, stride, offset );
	glEnableVertexAttribArray( index );
}

void GLBufferAddAttribute( GLBuffer* buffer, u32 index, u32 size, u32 type, i32 stride, const void* offset ) {
	//Int uses a slightly different format
	if ( type == GL_INT ) {
		GLBufferAddAttributeI( buffer, index, size, type, stride, offset );
		return;
	}

	glVertexAttribPointer( index, size, type, GL_FALSE, stride, offset );
	glEnableVertexAttribArray( index );
}

void GLBufferAddDefaultAttribs( GLBuffer* buffer ) {
	GLBufferAddAttribute( buffer, 0, 3, GL_FLOAT, sizeof( DrawVertex ), ( void* ) 0 );
	GLBufferAddAttribute( buffer, 1, 3, GL_FLOAT, sizeof( DrawVertex ), ( void* ) offsetof( DrawVertex, normal ) );
	GLBufferAddAttribute( buffer, 2, 2, GL_FLOAT, sizeof( DrawVertex ), ( void* ) offsetof( DrawVertex, tex ) );
}

void GLBufferAddDefaultSkinnedAttribs( GLBuffer* buffer ) {
	GLBufferAddAttribute( buffer, 3, 4, GL_FLOAT, sizeof( DrawVertex ), ( void* ) offsetof( DrawVertex, tangents ) );
	GLBufferAddAttributeI( buffer, 4, 4, GL_INT, sizeof( DrawVertex ), ( void* ) offsetof( DrawVertex, bones ) );
	GLBufferAddAttribute( buffer, 5, 4, GL_FLOAT, sizeof( DrawVertex ), ( void* ) offsetof( DrawVertex, weights ) );
}