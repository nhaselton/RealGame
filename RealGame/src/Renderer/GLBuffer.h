#pragma once
#include "def.h"

class GLBuffer {
public:
	u32 vao;
	u32 vbo;
	u32 ebo;

	//These don't get stored on the CPU anymore, these get stored on the GPU
	u32 numVertices;
	u32 numIndices;

	u32 vboSize;
	u32 eboSize;

	int pad;//32 bit aligned
};

void CreateGLBuffer( GLBuffer* buffer, u32 numVertices, u32 numIndices, u32 vboSize, void* vboData, u32 eboSize, void* eboData, bool staticVBO, bool staticEBO );
void GLBufferAddAttribute( GLBuffer* buffer, u32 index, u32 size, u32 type, i32 stride, const void* offset );
void GLBufferAddDefaultAttribs( GLBuffer* buffer );
void GLBufferAddDefaultSkinnedAttribs( GLBuffer* buffer );

void GLBufferAddDefaultAttribsStatic( GLBuffer* buffer );
void GLBufferAddDefaultAttribsSkinned( GLBuffer* buffer );
