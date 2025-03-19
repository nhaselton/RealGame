#include "def.h"
#include "game.h"
#include "renderer/DebugRenderer.h"
#include "Renderer/Renderer.h"

void SpawnTargetLoadKVP(void* _spawner, char* key, char* value);
void SpawnZoneLoadKVP(void* _zone, char* key, char* value);
void TriggerLoadKVP(void* _trigger, char* key, char* value);
void LightLoadKVP(void* _light, char* key, char* value);

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

//Adds the temp light to the correct array
void LightHack(Light* newEnt) {
	Light* light = (Light*)newEnt;

	if (light->isStatic) {
		if (renderer.worldView.numStaticLights < MAX_STATIC_LIGHTS)
			renderer.worldView.staticLights[renderer.worldView.numStaticLights++] = *light;
	}
	else {
		Light* newLight = NewLight();
		if (!newLight) {
			LOG_WARNING(LGS_GAME, "no more room for lights\n");
			return;
		}
		*newLight = *light;
	}
}

void SpawnZoneHack(SpawnTarget* spawner) {
	Vec3 min = spawner->pos;
	Vec3 max = spawner->size;
	spawner->pos = (max + min) / 2.0f;
	spawner->size = (max - min) / 2.0f;
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

	//The entity that was just created and it's function to read the KVP
	void* newEnt = 0;
	//This is the entity's ReadKeyValue function
	void (*EntKVP) (void* ent, char* key, char* value) = 0;


	while( parser.GetCurrent().type != TT_EOF ) {
		//Get Type of entity
		parser.ExpectedTokenTypePunctuation( '{' );
		parser.ExpectedTokenString( "classname" );

		char className[MAX_NAME_LENGTH]{};
		parser.ParseString( className, MAX_NAME_LENGTH );

		bool isLight = false;
		bool isSpawnZone = false;

		//TODO hash
		if( !strcmp( className, "info_player_start" ) ) {
			newEnt = CreatePlayer(Vec3(0));
			EntKVP = PlayerLoadKVP;
		}
		else if( !strcmp( className, "info_goblin_start" ) ) {
			newEnt = CreateGoblin(Vec3(0));
			EntKVP = GoblinLoadKVP;
		}
		else if( !strcmp( className, "info_wizard_start" ) ) {
			newEnt = (Wizard*) CreateWizard(Vec3(0));// WizardLoad(&parser);
			EntKVP = WizardLoadKeyValue;
		}
		else if( !strcmp( className, "info_ogre_start" ) ) {
			newEnt = (Ogre*)CreateOgre(Vec3(0));
			EntKVP = OgreLoadKVP;
		}
		else if (!strcmp(className, "info_chaingunner_start")) {
			newEnt = (Chaingunner*)CreateChaingunner(Vec3(0));
			EntKVP = ChaingunnerLoadKVP;
		}
		else if (!strcmp(className, "info_boar_start")) {
			newEnt = (Boar*)CreateBoar(Vec3(0));
			EntKVP = BoarLoadKVP;
		}
		else if( !strcmp( className, "trigger_once" ) ) {
			newEnt = &entityManager.triggers[entityManager.numTriggers++];
			EntKVP = TriggerLoadKVP;
			memset(newEnt, 0, sizeof(Trigger));
		}
		else if( !strcmp( className, "spawner" ) ) {
			newEnt = &entityManager.spawnTargets[entityManager.numSpawnTargets++];
			EntKVP = SpawnTargetLoadKVP;
			memset(newEnt, 0, sizeof(SpawnTarget));
			((SpawnTarget*)newEnt)->type = SPAWN_TARGET_POINT;

		}
		else if( !strcmp( className, "spawn_zone" ) ) {
			newEnt = &entityManager.spawnTargets[entityManager.numSpawnTargets++];
			EntKVP = SpawnZoneLoadKVP;
			memset(newEnt, 0, sizeof(SpawnTarget));
			((SpawnTarget*)newEnt)->type = SPAWN_TARGET_ZONE;
			isSpawnZone = true;
		}
		else if( !strcmp( className, "light" ) ) {
			//Because static and dynamci lights go in differnt locations,
			//This has to load as a temp light first then be placed in the correct array after
			EntKVP = LightLoadKVP;
			//Hack
			isLight = true;
		}
		else if( !strcmp( className, "door" ) ) {
			EntKVP = DoorLoadKVP;
			newEnt = NewEntity();
			{
				Entity* e = (Entity*) newEnt;
				e->renderModel = (RenderModel*) ScratchArenaAllocate( &globalArena, sizeof( RenderModel ) );
				memset( e->renderModel, 0, sizeof( *e->renderModel ) );
				e->renderModel->model = LoadModel( "res/models/door.glb" );
				e->renderModel->scale = Vec3( 1 );
				entityManager.door = (Door*) e;
			}
		}
		else if( !strcmp( className, "pickup_key" ) ) {
			char key[64];
			char value[64];
			Pickup* pickup = &entityManager.pickups[entityManager.numPickups++];
			LoadKeyValue( &parser, key, value );
			pickup->bounds.center = StringToVec3( value, true );
			pickup->bounds.width = Vec3( 1.5, 2.5, 1.5 );

			LoadKeyValue( &parser, key, value );
			int keyColor = atoi( value );
			LoadKeyValue( &parser, key, value );
			int isPickup = atoi( value ) == 1;

			if( isPickup ) {

				if( keyColor == 1 ) {
					pickup->flags = PICKUP_KEY_RED;
				}
				else {
					pickup->flags = PICKUP_KEY_BLUE;
				}
			}
			else {
				//pickup->renderModel.model = LoadModel( "res/models/gib.glb" );

				if( keyColor == 1 )
					pickup->flags = PICKUP_PLACE_KEY_RED;
				else
					pickup->flags = PICKUP_PLACE_KEY_BLUE;
			}

			if ( keyColor == 1 )
				pickup->renderModel.model = LoadModel( "res/models/keyred.glb" );
			else
				pickup->renderModel.model = LoadModel( "res/models/keyblue.glb" );

			pickup->renderModel.scale = Vec3( 0.5f );
			pickup->renderModel.rotation = Quat( 1, 0, 0, 0 );
			pickup->renderModel.translation = Vec3( 0, 0, 0 );
			parser.ExpectedTokenTypePunctuation( '}' );
			continue;
		}
		else if( !strcmp( className, "pickup_healthpack" ) ) {
			char key[64];
			char value[64];
			Pickup* pickup = &entityManager.pickups[entityManager.numPickups++];
			LoadKeyValue( &parser, key, value );
			pickup->bounds.center = StringToVec3( value, true );
			pickup->bounds.width = Vec3( 1, 0.5, 1 );
			pickup->renderModel.model = LoadModel( "res/models/healthpack.glb" );
			pickup->renderModel.scale = Vec3( 1,0.5,1 );
			pickup->flags = PICKUP_MEDKIT;
		}
		else if( !strcmp( className, "pickup_ammopack" ) ) {
			char key[64];
			char value[64];
			Pickup* pickup = &entityManager.pickups[entityManager.numPickups++];
			LoadKeyValue( &parser, key, value );
			pickup->bounds.center = StringToVec3( value, true );
			pickup->bounds.width = Vec3( 1, 1, 1 );
			pickup->renderModel.model = LoadModel( "res/models/ammopack.glb" );
			pickup->renderModel.scale = Vec3( 1, 1, 1 );
			pickup->flags = PICKUP_AMMOPACK;
		}
		else if (!strcmp(className, "pickup_weapon")) {
			char key[64];
			char value[64];
			Pickup* pickup = &entityManager.pickups[entityManager.numPickups++];
			LoadKeyValue(&parser, key, value);
			pickup->bounds.center = StringToVec3(value, true);
			pickup->bounds.width = Vec3(2, 1, 2);

			LoadKeyValue(&parser, key, value);
			int ivalue = atoi(value);
			pickup->flags = ivalue;

			switch (ivalue) {
				case PICKUP_REVOLVER: pickup->renderModel.model = LoadModel("res/models/Revolver.glb"); break;
				case PICKUP_SHOTGUN: pickup->renderModel.model = LoadModel("res/models/shotgun.glb"); break;
				case PICKUP_PLASMA: pickup->renderModel.model = LoadModel("res/models/plasma.glb"); break;
				case PICKUP_RPG: pickup->renderModel.model = LoadModel("res/models/rpg.glb"); break;
			}
			pickup->renderModel.scale = Vec3(1);
			pickup->renderModel.rotation = Quat(1, 0, 0, 0);
			pickup->renderModel.translation = Vec3(0, 1, 0);
			pickup->renderModel.scale = Vec3(2.0f);
			
			parser.ExpectedTokenTypePunctuation('}');
			continue;//SKip rest of loop
		}
		//Entity Does not exist, just skip over it
		else {
			LOG_WARNING( LGS_GAME, "Unkown classname %s\n", className );
			parser.SkipUntilTokenOfType( TT_PUNCTUATION, TS_RB );
		}

		//Load all KVP
		char key[64];
		char value[64];
		//Make sure we can actually load the entity
		bool validEntity = (newEnt && EntKVP);

		while (parser.GetCurrent().subType != '}') {
			LoadKeyValue(&parser, key, value);
			if (validEntity) {
				EntKVP(newEnt, key, value);
			}
		}
		parser.ExpectedTokenTypePunctuation('}');
		//Hack for light
		if (isLight) LightHack((Light*)newEnt);
		//Hack for spawnZone
		if (isSpawnZone) 
			SpawnZoneHack((SpawnTarget*)newEnt);
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

void TriggerLoadKVP(void* _trigger, char* key, char* value) {
	Trigger* trigger = (Trigger*)_trigger;
	if (TryBrushField(key, value, &trigger->bounds)) {

	}
	else if (TryTargetField(key, value, trigger->myName, trigger->willTrigger)) {

	}
	else if (!strcmp(key, "triggertype")) {
		trigger->type = (trigger_t)atoi(value);
	}
	else {
		LOG_WARNING(LGS_GAME, "trigger bad key value %s : %s\n", key, value);
	}
}

void SpawnTargetLoadKVP(void* _spawner, char* key, char* value) {
	SpawnTarget* spawner = (SpawnTarget*)_spawner;

	if (!strcmp(key, "origin")) {
		spawner->pos = StringToVec3(value, true);
	}
	else if (!strcmp(key, "targetname")) {
		strcpy(spawner->name, value);
	}
	else if (!strcmp(key, "types")) {
		spawner->enemies = (encounterEnemies_t)atoi(value);
	}
	else {
		LOG_WARNING(LGS_GAME, "spawner bad key value %s : %s\n", key, value);
	}
}

void SpawnZoneLoadKVP(void* _spawner, char* key, char* value) {
	SpawnTarget* spawner = (SpawnTarget*)_spawner;
	//spawner bounds
	BoundsMinMax bounds{};
	char buffer[MAX_NAME_LENGTH];
	
	if (TryBrushField(key, value, &bounds)) {

	}
	else if (TryTargetField(key, value, spawner->name, buffer)) {

	}
	else if (!strcmp(key, "types")) {
		spawner->enemies = (encounterEnemies_t)atoi(value);
	}
	else {
		LOG_WARNING(LGS_GAME, "trigger bad key value %s : %s\n", key, value);
	}

	//Hack since I load in 1 key value at a time
	if (bounds.min != Vec3(0))
		spawner->pos = bounds.min;
	if (bounds.max != Vec3(0))
		spawner->size = bounds.max;
}

void LightLoadKVP(void* _light, char* key, char* value) {
	Light* light = (Light*)_light;
	light->type = LIGHT_POINT;
	light->color = Vec3(1);
	light->cutoff = glm::cos(glm::radians(20.0f));
	light->intensity = 1;
	light->isStatic = true;
	LightSetAttenuation(light, 65);

	//Light* light = (Light*)_light;
	Light lightTemp{};

	if (!strcmp(key, "origin")) {
		light->pos = StringToVec3(value, true);
	}
	else if (!strcmp(key, "type")) {
		light->type = (float)(atoi(value));
	}
	//Should set float (0-1)
	else if (!strcmp(key, "_color")) {
		light->color = StringToVec3(value, false);
	}
	else if (!strcmp(key, "attenuation")) {
		LightSetAttenuation(light, atoi(value));
	}
	else if (!strcmp(key, "intensity")) {
		light->intensity = atof(value);
	}
	else if (!strcmp(key, "direction")) {
		light->dir = StringToVec3(value, 0);
	}
	else if (!strcmp(key, "static")) {
		light->isStatic = !atoi(value);
	}
	else {
		LOG_WARNING(LGS_GAME, "spawner bad key value %s : %s\n", key, value);
	}
}
