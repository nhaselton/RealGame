#include "Level.h"
#include "Renderer\Renderer.h"
#include "Physics\Physics.h"
#include "Game/Game.h"

void CreateLevel( Level* level, void* memory, u32 size ) {
	CreateScratchArena( &level->arena, size, memory, &globalArena, "Level" );
}


void LoadLevel( Level* level, const char* path ) {
	TEMP_ARENA_SET;
	
	NFile file;
	CreateNFile( &file, path, "rb" );

	if ( !file.open ) {
		LOG_ASSERT( LGS_GAME, "Could not open level file %s\n", path );
		return;
	}

	RenderLoadLevel( level, &file );
	PhysicsLoadLevel( level, &file );

	char entityPath[MAX_PATH_LENGTH]{};
	int len = strlen( path );

	memcpy( entityPath, path, len - 3 );
	entityPath[len - 3] = 'e';
	entityPath[len - 2] = 'n';
	entityPath[len - 1] = 't';
	GameLoadEntities( entityPath );
}
