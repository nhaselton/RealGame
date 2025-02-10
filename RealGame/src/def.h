#pragma once
#include "core/coredef.h"
#include "Core/NMemory.h"
#include "core/Timer.h"
#include "core/Log.h"
#include "core/Window.h"
#include "Core\Input.h"
#include "Core\IO.h"
#include "Core/Parser.h"

#define TOTAL_MEMORY (MB(128))
#define TEMP_MEMORY (MB(25))

#define MAX_MODELS (100)
#define MAX_MODEL_SIZE (KB(32))
#define MODEL_MANAGER_SIZE  ( MAX_MODEL_SIZE * MAX_MODELS )

#define MAX_SHADERS 16
#define MAX_SHADER_ARGS ( MAX_SHADERS * 16 )

#define MAX_SHADER_ARG_SIZE ( 64 )
#define DEBUG_RENDERER_SIZE (MAX_LINES_BATCH * NUM_LINE_BATCHES * sizeof( DebugLine ) + sizeof( DebugPrim ) * MAX_PRIMS)

#define MAX_SOUND_BUFFERS 128
#define MAX_AUDIO_SOURCES 128

#define MAX_TEXTURES 128

#define MAX_ANIMATION_SIZE KB(128)
#define MAX_ANIMATIONS 50
#define ANIMATION_MANAGER_SIZE ( MAX_ANIMATION_SIZE * MAX_ANIMATIONS )

#define MAX_BRUSHES 4096
#define MAX_LEVEL_VERTICES (MAX_BRUSHES * 16)
#define MAX_LEVEL_INDICES (MAX_BRUSHES * 64)
#define LEVEL_MEMORY (MB(10))

#define MAX_ENTITIES 250
#define MAX_ENTITY_SIZE 2048
#define MAX_PROJECTILES 1000
#define MAX_RIGIDBODIES 500


#define PHYSICS_MEMORY MB(10)


//How big name buffers are when read from disk
#define NAME_BUF_LEN (MAX_NAME_LENGTH + 4)

//Defined in main.cpp
extern ScratchArena globalArena;
extern StackArena tempArena;
extern float dt;
extern float gameTime;

#define RED		Vec3( 1, 0, 0 ) 
#define GREEN	Vec3( 0, 1, 0 ) 
#define BLUE	Vec3( 0, 0, 1 ) 
#define BLACK	Vec3( 0 )
#define WHITE	Vec3( 1 )