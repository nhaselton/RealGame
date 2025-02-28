#pragma once
#include "core/coredef.h"
#include "Core/NMemory.h"
#include "core/Timer.h"
#include "core/Log.h"
#include "core/Window.h"
#include "Core\Input.h"
#include "Core\IO.h"
#include "Core/Parser.h"
#include "Core/Console.h"

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
#define MAX_STATIC_LIGHTS 512
#define MAX_DYNAMIC_LIGHTS 512

#define MAX_BONES 64
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
#define MAX_TRIGGERS 1024
#define MAX_SPAWN_TARGETS 128
#define MAX_ENCOUNTERS 128

#define PHYSICS_MEMORY MB(10)

//For entity compilier
//These are the things that get seralized
#define ENT_CLASS class
#define ENT_STRUCT struct
#define EVAR //This is for literals, ints, vec3s, etc. (No structs. we are too poor for that)
#define EPATH //This is for items that need a path to load. models, sounds, etc.
//This is a hack because the parser doesnt let you define custom names but TB forces you to use origin
#define ESTRUCT //This thing Can't exactly look through structs, so all variables come from name.EARG

#define ENT_RENAME(fileName)
#define ENT_PARENT(parent)


//How big name buffers are when read from disk
#define NAME_BUF_LEN (MAX_NAME_LENGTH + 4)

//Defined in main.cpp
extern ScratchArena globalArena;
extern StackArena tempArena;
extern float dt;
extern float gameTime;
extern int maxFps;

#define LIGHTGRAY  (Vec4{ 200, 200, 200, 255 } / 255.0f)  // Light Gray
#define GRAY       (Vec4{ 130, 130, 130, 255 } / 255.0f)  // Gray
#define DARKGRAY   (Vec4{ 80, 80, 80, 255 }    / 255.0f)  // Dark Gray
#define YELLOW     (Vec4{ 253, 249, 0, 255 }   / 255.0f)  // Yellow
#define GOLD       (Vec4{ 255, 203, 0, 255 }   / 255.0f)  // Gold
#define ORANGE     (Vec4{ 255, 161, 0, 255 }   / 255.0f)  // Orange
#define PINK       (Vec4{ 255, 109, 194, 255 } / 255.0f)  // Pink
#define RED        (Vec4{ 230, 41, 55, 255 }   / 255.0f)  // Red
#define MAROON     (Vec4{ 190, 33, 55, 255 }   / 255.0f)  // Maroon
#define GREEN      (Vec4{ 0, 228, 48, 255 }    / 255.0f)  // Green
#define LIME       (Vec4{ 0, 158, 47, 255 }    / 255.0f)  // Lime
#define DARKGREEN  (Vec4{ 0, 117, 44, 255 }    / 255.0f)  // Dark Green
#define SKYBLUE    (Vec4{ 102, 191, 255, 255 } / 255.0f)  // Sky Blue
#define BLUE       (Vec4{ 0, 121, 241, 255 }   / 255.0f)  // Blue
#define DARKBLUE   (Vec4{ 0, 82, 172, 255 }    / 255.0f)  // Dark Blue
#define PURPLE     (Vec4{ 200, 122, 255, 255 } / 255.0f)  // Purple
#define VIOLET     (Vec4{ 135, 60, 190, 255 }  / 255.0f)  // Violet
#define DARKPURPLE (Vec4{ 112, 31, 126, 255 }  / 255.0f)  // Dark Purple
#define BEIGE      (Vec4{ 211, 176, 131, 255 } / 255.0f)  // Beige
#define BROWN      (Vec4{ 127, 106, 79, 255 }  / 255.0f)  // Brown
#define DARKBROWN  (Vec4{ 76, 63, 47, 255 }    / 255.0f)  // Dark Brown
#define BLACK (Vec4(0))

inline void CopyPathAndChangeExtension( char* dst, const char* source, const char* newExt, int dstSize ) {
	int len = strlen( source );
	if( len >= dstSize ) {
		LOG_ASSERT( LGS_CORE, "%s is too big to copy into %d bytes", source, dst );
		return;
	}

	memcpy( dst, source, len - 3 );
	dst[len - 3] = newExt[0];
	dst[len - 2] = newExt[1];
	dst[len - 1] = newExt[2];


}
inline u64 HashStringBad( const char* s ) {
	char* s2 = ( char* ) s;

	u64 sum = 1;
	while( *s2 != '\0' ) {
		sum += *s2++ * sum;
	}
	return sum;
}

bool TempDumpFile( const char* path, char** buffer, u32* outLen );