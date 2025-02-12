#include "def.h"
#include "game.h"
#include "renderer/DebugRenderer.h"

Vec3 StringToVec3( const char* value ) {
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

		entity->pos += StringToVec3( value );
		entity->bounds->offset = entity->pos;
		return true;
	}
	return false;
}

bool TryTriggerField( Trigger* trigger, const char* key, const char* value ) {
	if( !strcmp( key, "target" ) ) {
		strcpy( trigger->target, value );
		return true;
	}

	if( !strcmp( key, "targetname" ) ) {
		strcpy( trigger->targetName, value );
		return true;
	}

	if( !strcmp( key, "bounds" ) ) {
		return true;
	}

	if( !strcmp( key, "boundsmin" ) ) {
		trigger->bounds.min = StringToVec3( value );
		return true;
	}

	if( !strcmp( key, "boundsmax" ) ) {
		trigger->bounds.max= StringToVec3( value );
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
		Trigger trigger{};
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
		else if( !strcmp( className, "trigger_once" ) ) {
			//Trigger
			isTrigger = true;
		}

		//Find all fields
		while( 1 ) {
			char key[MAX_NAME_LENGTH]{};
			char value[MAX_NAME_LENGTH]{};

			parser.ParseString( key, MAX_NAME_LENGTH );
			parser.ParseString( value, MAX_NAME_LENGTH );

			if( isTrigger ) {
				if( !TryTriggerField( &trigger, key, value ) ) {
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

}
