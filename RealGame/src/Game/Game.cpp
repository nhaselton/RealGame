#include "def.h"
#include "game.h"
#include "renderer/DebugRenderer.h"

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

		bool isTrigger = false;
		bool isSpawner = false;
		Trigger* trigger = 0;
		Entity* entity = 0;

		//Todo find better way to do this later
		if( !strcmp( className, "info_player_start" ) ) {
			entity = CreatePlayer( Vec3( 0 ) );
			entityManager.player = (Player*) entity;
		}
		else if( !strcmp( className, "info_goblin_start" ) ) {
			entity = CreateGoblin( Vec3( 0, 0, 0 ) );
		}
		else if( !strcmp( className, "info_wizard_start" ) ) {
			entity = CreateWizard( Vec3( 0, -1.5, 0 ) );
		}
		else if( !strcmp( className, "encounter" ) ) {
			//Origin
			char key0[MAX_NAME_LENGTH]{};
			char value0[MAX_NAME_LENGTH]{};
			parser.ParseString( key0, MAX_NAME_LENGTH );
			parser.ParseString( value0, MAX_NAME_LENGTH );
			//Name
			char key1[MAX_NAME_LENGTH]{};
			char value1[MAX_NAME_LENGTH]{};
			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );
			//strcpy( entityManager.encounters[0].name, value1 );
			parser.ExpectedTokenTypePunctuation( '}' );

			isSpawner = true;
		}
		else if( !strcmp( className, "spawner" ) ) {
			SpawnTarget* spawner = &entityManager.spawnTargets[entityManager.numSpawnTargets++];
			isSpawner = true;

			char key1[MAX_NAME_LENGTH]{};
			char value1[MAX_NAME_LENGTH]{};
			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );
			char key2[MAX_NAME_LENGTH]{};
			char value2[MAX_NAME_LENGTH]{};
			parser.ParseString( key2, MAX_NAME_LENGTH );
			parser.ParseString( value2, MAX_NAME_LENGTH );

			if ( parser.PeekNext().subType == '}' )
				parser.ExpectedTokenTypePunctuation( '}' );
			else {
				parser.ParseString( key2, MAX_NAME_LENGTH );
				//parser.ParseString( value2, MAX_NAME_LENGTH );
				spawner->enemies = (encounterEnemies_t) parser.ParseInt();
				parser.ExpectedTokenTypePunctuation( '}' );
			}

			spawner->type = SPAWN_TARGET_POINT;
			spawner->pos = StringToVec3( value1, true );
			strcpy( spawner->name, value2 );
		}
		else if( !strcmp( className, "trigger_once" ) ) {
			//Trigger
			trigger = &entityManager.triggers[entityManager.numTriggers++];
			isTrigger = true;
		}
		else if( !strcmp( className, "spawn_zone" ) ) {
			SpawnTarget* spawner = &entityManager.spawnTargets[entityManager.numSpawnTargets++];

			//Bounds True
			char key1[MAX_NAME_LENGTH]{};
			char value1[MAX_NAME_LENGTH]{};
			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );
			strcpy( spawner->name, value1 );

			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );

			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );
			Vec3 min = StringToVec3( value1, false );

			parser.ParseString( key1, MAX_NAME_LENGTH );
			parser.ParseString( value1, MAX_NAME_LENGTH );
			Vec3 max = StringToVec3( value1, false );

			spawner->type = SPAWN_TARGET_ZONE;
			spawner->pos = ( max + min ) / 2.0f;
			spawner->size = ( max - min ) / 2.0f;
			
			isSpawner = true;
			parser.ExpectedTokenTypePunctuation( '}' );

		}
		else {
			printf( "Unkown Entity type %s\n", className );
			__debugbreak();
		}

		//Find all fields
		while( 1 ) {
			if( isSpawner  )
				break;

			char key[MAX_NAME_LENGTH]{};
			char value[MAX_NAME_LENGTH]{};

			parser.ParseString( key, MAX_NAME_LENGTH );
			parser.ParseString( value, MAX_NAME_LENGTH );

			if( isTrigger ) {
				if( !TryTriggerField( trigger, key, value ) ) {
					LOG_WARNING( LGS_GAME, "Unkown Trigger Key Value %s \n", key );
				}
			}
			else {
				if( !TryEntityField( entity, key, value ) ) {
					LOG_WARNING( LGS_GAME, "Unkown Enity Key Value %s \n", key );
				}
			}

			if( parser.GetCurrent().subType == '}' ) {
				parser.ReadToken();
				break;
			}
		}
	}

	NFileClose( &file );
}
