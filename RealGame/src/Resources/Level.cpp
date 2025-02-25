#include "Level.h"
#include "Renderer\Renderer.h"
#include "Physics\Physics.h"
#include "Game/Game.h"

void CreateLevel( Level* level, void* memory, u32 size ) {
	CreateScratchArena( &level->arena, size, memory, &globalArena, "Level" );
}


bool LoadLevel( Level* level, const char* path ) {
	TEMP_ARENA_SET;
	

	NFile file;
	CreateNFile( &file, path, "rb" );

	if ( !file.open ) {
		LOG_ERROR( LGS_GAME, "Could not open level file %s\n", path );
		return 0;
	}
	memset( level->path, 0, MAX_PATH_LENGTH );
	strcpy( level->path, path );

	RenderLoadLevel( level, &file );
	PhysicsLoadLevel( level, &file );

	char entityPath[MAX_PATH_LENGTH]{};
	int len = strlen( path );

	memcpy( entityPath, path, len - 3 );
	entityPath[len - 3] = 'e';
	entityPath[len - 2] = 'n';
	entityPath[len - 1] = 't';
	GameLoadEntities( entityPath );

	CreateEncounters();
	return 1;
}

void UnloadLevel( Level* level ) {
	level->numBrushes = 0;
	level->numFaces = 0;
	level->numIndices = 0;
	level->numVertices = 0;
	ScratchArenaFree( &level->arena );

	RenderUnloadLevel();
	PhysicsUnloadLevel();
	GameUnloadLevel();
}

extern Level level;
void ConsoleChangeLevel() {
	if( console.cvarArgc != 1 ) {
		console.WriteLine( "bad syntax. expected map [path]" );
		return;
	}

	char buffer[128]{};
	strcpy( buffer, "res/maps/" );
	strcat( buffer, console.cvarArgv[0].data );
	strcat( buffer, ".cum" );


	UnloadLevel( &level );
	if( !LoadLevel( &level, buffer ) ) {
		LOG_ERROR( LGS_GAME, "Could not load level %s\n", buffer );

		if( !LoadLevel( &level, "res/maps/blank.cum" ) ) {
			LOG_ASSERT( LGS_GAME, "CAN NOT LOAD BLANK MAP\n" );
		}
	}
}