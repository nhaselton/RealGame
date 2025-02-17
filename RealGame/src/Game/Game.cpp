#include "def.h"
#include "game.h"
#include "renderer/DebugRenderer.h"

void LoadTrigger( Parser* parser );
void LoadSpawner( Parser* parser );

Vec3 StringToVec3( const char* value, bool fix ) {
	Vec3 v(0);
	int len = strlen( value );

	//Offset into string
	int o = 0;
	for( int i = 0; i < 3; i++ ) {
		//Start of this number
		int start = o;
		while( value[o] != ' ' && o != len ) {
			o++;
		}
		//Need to put it in a buffer first because you cant atof on part of a string
		char buffer[32]{};
		memcpy( buffer, value + start, o - start );
		v[i] = atof( buffer );
		//move past space
		o++;
	}
	if ( fix )
		v = Vec3( -v.x, v.z, v.y ) / 32.0f;
	return v;
}

//Tries to set a base entitiy's attribute. if found will set and return true
//Else return false
bool TryEntityField( Entity* entity, const char* key, const char* value ) {
	if( !strcmp( key, "origin" ) ) {
		//All entity positions are set to their correct offset,
		//so adding the new position allows for per entity offsets so their feet are on the ground
		Vec3 pos( 0 );

		entity->pos += StringToVec3( value, true );
		entity->bounds->offset = entity->pos;
		return true;
	}
	return false;
}

bool TryTriggerField( Trigger* trigger, const char* key, const char* value ) {
	if( !strcmp( key, "target" ) ) {
		strcpy( trigger->willTrigger, value );
		return true;
	}

	if( !strcmp( key, "targetname" ) ) {
		strcpy( trigger->myName, value );
		return true;
	}

	if( !strcmp( key, "bounds" ) ) {
		return true;
	}

	if( !strcmp( key, "triggertype" ) ) {
		trigger->type = ( trigger_t ) atoi( value );
		return true;
	}

	if( !strcmp( key, "boundsmin" ) ) {
		trigger->bounds.min = StringToVec3( value, false );
		return true;
	}

	if( !strcmp( key, "boundsmax" ) ) {
		trigger->bounds.max = StringToVec3( value, false );
		return true;
	}
	return false;
}

void GameLoadEntities( const char* path ) {
	TEMP_ARENA_SET;

	NFile file;
	CreateNFile( &file, path, "rb" );

	if( !file.file ) {
		return;
	}

	char* buffer = ( char* ) TEMP_ALLOC( file.length + 1);
	buffer[file.length] = '\0';


	NFileRead( &file, buffer, file.length + 1);
	NFileClose( &file );

	Parser parser( buffer, file.length );
	parser.ReadToken();

	while( parser.GetCurrent().type != TT_EOF ) {
		//Get Type of entity
		parser.ExpectedTokenTypePunctuation( '{' );
		parser.ExpectedTokenString( "classname" );

		char className[MAX_NAME_LENGTH]{};
		parser.ParseString( className, MAX_NAME_LENGTH );

		//TODO hash
		if( !strcmp( className, "info_player_start" ) ) {
			PlayerLoad(&parser);
		}
		else if( !strcmp( className, "info_goblin_start" ) ) {
			GoblinLoad( &parser );
		}
		else if( !strcmp( className, "info_wizard_start" ) ) {
			WizardLoad( &parser );
		}
		else if( !strcmp( className, "trigger_once" ) ) {
			LoadTrigger(&parser);
		}
		else if( !strcmp( className, "spawner" ) ) {
			LoadSpawner( &parser );
		}


		//Entity Does not exist, just skip over it
		else {
			LOG_WARNING( LGS_GAME, "Unkown classname %s\n", className );
			parser.SkipUntilTokenOfType( TT_PUNCTUATION, TS_RB );
			parser.ReadToken();
		}

	}
}

void GameUnloadLevel() {
	entityManager.numEncounters = 0;
	entityManager.numEntities = 0;
	entityManager.numProjectiles = 0;
	entityManager.numRemoveEntities = 0;
	entityManager.numRemoveEntities = 0;
	entityManager.numTriggers = 0;
	entityManager.lastProjectileIndex = 0;
	entityManager.player = 0;
	entityManager.numSpawnTargets = 0;

	memset( &entityManager, 0, sizeof( EntityManager ) );

}

void LoadTrigger( Parser* parser ) {
	if( entityManager.numTriggers == MAX_TRIGGERS ) {
		LOG_ASSERT( LGS_GAME, "CAN NOT ADD ANOTHER TRIGGER\n" );
		return;
	}

	Trigger* trigger = &entityManager.triggers[entityManager.numTriggers++];
	memset( trigger, 0, sizeof( Trigger ) );

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		if( !strcmp( key, "bounds" ) ) {
			//does nothing
		}
		else if( !strcmp( key, "boundsmin" ) ) {
			trigger->bounds.min = StringToVec3( value, false );
		}
		else if( !strcmp( key, "boundsmax" ) ) {
			trigger->bounds.max = StringToVec3( value, false );
		}
		else if( !strcmp( key, "targetname" ) ) {
			strcpy( trigger->myName, value );
		}
		else if( !strcmp( key, "target" ) ) {
			strcpy( trigger->willTrigger, value );
		}
		else if( !strcmp( key, "triggertype" ) ) {
			trigger->type = (trigger_t) atoi( value );
		}

		else {
			LOG_WARNING( LGS_GAME, "trigger bad key value %s : %s\n", key, value );
		}


		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}

	if( trigger->myName[0] == '\0' ) {
		LOG_WARNING( LGS_GAME, "Warning trigger forgot name\n" );
	}

	if( trigger->willTrigger[0] == '0' ) {
		if( trigger->myName[0] == '\0' ) {
			LOG_WARNING( LGS_GAME, "trigger forgot name and target" );
			return;
		}
		LOG_WARNING( LGS_GAME, "Warning trigger %s forgot target\n", trigger->myName );
	}
}

void LoadSpawner( Parser* parser ) {
	if( entityManager.numSpawnTargets == MAX_SPAWN_TARGETS) {
		LOG_ASSERT( LGS_GAME, "CAN NOT ADD ANOTHER SPAWN TARGET\n" );
		return;
	}

	SpawnTarget* spawner = &entityManager.spawnTargets[entityManager.numSpawnTargets++];
	spawner->type = SPAWN_TARGET_POINT;
	memset( spawner , 0, sizeof( SpawnTarget ) );

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		
		if( !strcmp( key, "origin" ) ) {
			spawner->pos = StringToVec3( value, true );
		}else if( !strcmp( key, "targetname" ) ) {
				strcpy( spawner->name, value );
		}
		else if( !strcmp( key, "types" ) ) {
			spawner->enemies = ( encounterEnemies_t ) atoi( value );
		}
		else {
			LOG_WARNING( LGS_GAME, "spawner bad key value %s : %s\n", key, value );
		}


		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}

	if( spawner->name[0] == '\0' ) {
		LOG_WARNING( LGS_GAME, "Forgot to set spawner name" );
	}
}