#include "def.h"
#include "game.h"
#include "renderer/DebugRenderer.h"
#include "Renderer/Renderer.h"
#include "../../EntC/gen/GEN_Light.h"

void LoadTrigger( Parser* parser );
void LoadSpawner( Parser* parser );
void LoadSpawnZone( Parser* parser );
void LoadLight( Parser* parser );

void KillAI() {
	for( int i = 0; i < entityManager.numEntities; i++ ) {
		Entity* e = &entityManager.entities[i].entity;
		if( e != entityManager.player )
			RemoveEntity( e );
	}
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

bool TryBrushField( const char* key, const char* value, BoundsMinMax* bounds ) {
	if( !strcmp( key, "bounds" ) ) {
		return true;
	}

		if( !strcmp( key, "boundsmin" ) ) {
		bounds->min = StringToVec3( value, false );
		return true;
	}

	if( !strcmp( key, "boundsmax" ) ) {
		bounds->max = StringToVec3( value, false );
		return true;
	}
	return false;
}

bool TryTargetField( char* key, char* value, char outName[MAX_NAME_LENGTH], char outTarget[MAX_NAME_LENGTH] ) {
		if( !strcmp( key, "targetname" ) ) {
			strcpy( outName, value );
			return true;
		}
		else if( !strcmp( key, "target" ) ) {
			strcpy( outTarget, value );
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
		else if( !strcmp( className, "info_ogre_start" ) ) {
			OgreLoad( &parser );
		}
		else if( !strcmp( className, "trigger_once" ) ) {
			LoadTrigger(&parser);
		}
		else if( !strcmp( className, "spawner" ) ) {
			LoadSpawner( &parser );
		}
		else if( !strcmp( className, "spawn_zone" ) ) {
			LoadSpawnZone(&parser);
		}
		else if( !strcmp( className, "light" ) ) {
			LoadLight(&parser);
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

		if( TryBrushField( key, value, &trigger->bounds ) ) {

		}
		else if( TryTargetField(key, value, trigger->myName, trigger->willTrigger) ) {

		}
		else if( !strcmp( key, "triggertype" ) ) {
			trigger->type = ( trigger_t ) atoi( value );
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
	memset( spawner , 0, sizeof( SpawnTarget ) );
	spawner->type = SPAWN_TARGET_POINT;

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

void LoadSpawnZone( Parser* parser ) {
	if( entityManager.numTriggers == MAX_TRIGGERS ) {
		LOG_ASSERT( LGS_GAME, "CAN NOT ADD ANOTHER TRIGGER\n" );
		return;
	}

	SpawnTarget* spawner = &entityManager.spawnTargets[entityManager.numSpawnTargets++];
	memset( spawner, 0, sizeof( SpawnTarget ) );
	spawner->type = SPAWN_TARGET_ZONE;

	//spawner bounds
	BoundsMinMax bounds{};

	char buffer[MAX_NAME_LENGTH];
	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );



		if( TryBrushField( key, value, &bounds) ) {

		}
		else if( TryTargetField( key, value, spawner->name, buffer ) ) {

		}
		else if( !strcmp( key, "types" ) ) {
			spawner->enemies = ( encounterEnemies_t ) atoi( value );
		}
		else {
			LOG_WARNING( LGS_GAME, "trigger bad key value %s : %s\n", key, value );
		}


		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}

	spawner->pos = ( bounds.max + bounds.min ) / 2.0f;
	spawner->size = (bounds.max - bounds.min) / 2.0f;

	if( spawner->name[0] == '\0' ) {
		LOG_WARNING( LGS_GAME, "Forgot to set spawner name" );
	}
}

void LoadLight( Parser* parser ) {
	//Hack
	Light backup;

	//Light* light = NewLight();
	Light light{};
	light.type = LIGHT_POINT;
	light.color = Vec3( 1 );
	light.cutoff = glm::cos( glm::radians( 20.0f ) );
	light.intensity = 1;
	light.isStatic = true;
	LightSetAttenuation( &light, 50 );

	while( 1 ) {
		char key[MAX_NAME_LENGTH]{};
		char value[MAX_NAME_LENGTH]{};

		parser->ParseString( key, MAX_NAME_LENGTH );
		parser->ParseString( value, MAX_NAME_LENGTH );

		if( !SetLightSwitch_GENERATED( &light, key, value ) ) {
			if( !strcmp( key, "attenuation" ) ) {
				LightSetAttenuation( &light, atoi( value ) );
			}
			else {
				LOG_WARNING( LGS_GAME, "spawner bad key value %s : %s\n", key, value );
			}
		}

		if( parser->GetCurrent().subType == '}' ) {
			parser->ReadToken();
			break;
		}
	}
	
	//Sucks to have to copy it after creation, but no way to tell if its a light or not
	if( light.isStatic ) {
		if( renderer.worldView.numStaticLights == MAX_STATIC_LIGHTS )
			return;
		renderer.worldView.staticLights[renderer.worldView.numStaticLights++] = light;
	}
	else {
		Light* newLight = NewLight();
		if( !newLight ) {
			LOG_WARNING( LGS_GAME, "no more room for lights\n" );
			return;
		}
		*newLight = light;
	}
}
