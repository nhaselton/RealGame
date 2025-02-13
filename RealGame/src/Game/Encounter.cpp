#include "encounter.h"
#include "Game.h"
#include "EntityManager.h"

SpawnTagGroup* FindSpawnTagGroup( Encounter* encounter, const char* name ) {
	for( int i = 0; i < encounter->numSpawnTags; i++ ) {
		if( !strcmp( encounter->spawnTags[i].name, name ) ) {
			return &encounter->spawnTags[i];
		}
	}
}

SpawnTagGroup* FindOrCreateSpawnTagGroup( Encounter* encounter, const char* name ) {
	if( name[0] == '\0' )
		return 0;

	SpawnTagGroup* spawnTag = FindSpawnTagGroup( encounter, name );
	if( !spawnTag ) {
		spawnTag = &encounter->spawnTags[encounter->numSpawnTags++];
		spawnTag->killed = 0;
		spawnTag->spawned = 0;
		strcpy( spawnTag->name, name );
	}

	return spawnTag;
}

void EncounterNotifyOnDeath( Encounter* encounter, Entity* entity ) {
	//Safety, entity should probably do this instead
	if( !encounter || entity->spawnTag[0] == '\0' )
		return;

	//Find action with correct Spawntag
	SpawnTagGroup* group = FindSpawnTagGroup( encounter, entity->spawnTag );

	if( !group ) {
		LOG_ERROR( LGS_GAME, "SPAWN TAG GROUP DOES NOT EXIST %S\n", entity->spawnTag );
	}
	else {
		group->killed++;
	}
}

void SpawnEnemy( Encounter* encounter, encounterEnemies_t enemy, Vec3 pos, SpawnTagGroup* spawnGroup ) {
	Entity* e = 0; 
	switch( enemy ) {
		case ENCOUNTER_AI_GOBLIN: e = CreateGoblin( pos ); break;
		case ENCOUNTER_AI_WIZARD: e = CreateWizard( pos ); break;
		default:
		LOG_WARNING( LGS_GAME, "Trying to spawn Enemy type %d that does not exist\n", enemy );
	}

	if( spawnGroup != 0 ) {
		strcpy( e->spawnTag, spawnGroup->name );
		spawnGroup->spawned++;
		e->encounter = encounter;
	}

}

inline Vec3 PosFromSpawnZone( SpawnTarget* target ) {
	Vec3 out{};
	for( int i = 0; i < 3; i++ ) {
		float LO = target->pos[i] - target->size[i];
		float HI = target->pos[i] + target->size[i];
		float r3 = LO + static_cast < float > ( rand() ) / ( static_cast < float > ( RAND_MAX / ( HI - LO ) ) );
		out[i] = r3;
	}
	return out;
}

int EncounterSpawnMultiAICalculateSpawnCount( const EncounterAction* action ) {
	if( action->spawnRate == 0.0f ) {
		return action->spawnCount;
	}
	
	int current = 1 + ( action->currentTime / action->spawnRate );
	if( current > action->spawnCount )
		return 0;

	return current - action->numSpawned;
}

SpawnTarget* FindSpawnTarget( const char* name ) {
	for( int n = 0; n < entityManager.numSpawnTargets; n++ ) {
		if( !strcmp( name, entityManager.spawnTargets[n].name ) )
			return &entityManager.spawnTargets[n];
	}
	return 0;
}

void EncounterAddActions( Encounter* encounter ) {
	for( int i = encounter->nextAction; i < encounter->totalActions; i++, encounter->nextAction++ ) {
		if( encounter->block )
			return;

		EncounterAction* action = &encounter->actions[i];
		switch( action->type ) {
			case ENCOUNTER_ACTION_SPAWN_SINGLE_AI: {
				printf( "Encounter Spawn AI\n" );
				SpawnTarget* target = FindSpawnTarget( action->spawnTarget );
				SpawnTagGroup* group = FindOrCreateSpawnTagGroup( encounter, action->spawnTag );

				if( !target ) {
					LOG_WARNING( LGS_GAME, "Encounter could not spawn enemy at %s\n", action->spawnTarget );
					break;
				}

				Vec3 spawnPos{};
				if( target->type == SPAWN_TARGET_POINT )
					spawnPos = target->pos;
				else
					spawnPos = PosFromSpawnZone( target );


				SpawnEnemy( encounter, action->ai, spawnPos, group );
			}break;
			case ENCOUNTER_ACTION_SPAWN_MULTIPLE_AI: {
				printf( "Encounter Spawn Multiple AI\n" );
				SpawnTarget* target = FindSpawnTarget( action->spawnTarget );
				SpawnTagGroup* group = FindOrCreateSpawnTagGroup( encounter, action->spawnTag );
				if( !target ) {
					LOG_WARNING( LGS_GAME, "Encounter could not spawn enemy at %s\n", action->spawnTarget );
					break;
				}
				Vec3 spawnPos{};
				
				if( target->type == SPAWN_TARGET_POINT ) {
					LOG_WARNING( LGS_GAME, "currently Using ENCOUNTER_ACTION_SPAWN_MULTIPLE_AI on a spawnPoint and NOT a spawnZone" );
					spawnPos = target->pos;
				}
				action->activeSpawnTarget = target;
				int toSpawn = EncounterSpawnMultiAICalculateSpawnCount( action );
				action->numSpawned += toSpawn;

				//If there are more left to spawn add it active list
				if( action->numSpawned < action->spawnCount ) {
					encounter->activeActions[encounter->numActiveActions++] = action;
					action->active = true;
				}
				for( int n = 0; n < toSpawn; n++ ) {
					spawnPos = PosFromSpawnZone( target );
					SpawnEnemy( encounter, action->ai, spawnPos, group );
				}
			}break;
			case ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK:
			{
				printf( "Encounter Wait For Seconds\n" );
				encounter->block = action;
				action->waitTime += gameTime;
			}break;
			case ENCOUNTER_ACTION_WAIT_FOR_SPAWN_GROUP_DEAD_BLOCK:
			{
				printf( "Encounter Wait Spawn Group Dead\n" );
				encounter->block = action;
			}break;

			default:
			{
				printf( "Bad Action %d\n", encounter->nextAction);
			}
		}
	}
}

void StartEncounter( Encounter* encounter ) {
	encounter->active = true;
	EncounterAddActions( encounter );
}

void UpdateEncounter( Encounter* encounter ) {
	if( !encounter->active )
		return;
	
	//Update actions
	for( int i = 0; i < encounter->numActiveActions; i++ ) {
		EncounterAction* action = encounter->activeActions[i];
		action->currentTime += dt;

		switch( action->type ) {
			case ENCOUNTER_ACTION_SPAWN_MULTIPLE_AI: {
				SpawnTagGroup* group = FindOrCreateSpawnTagGroup( encounter, action->spawnTag );
				int toSpawn = EncounterSpawnMultiAICalculateSpawnCount( action );
				action->numSpawned += toSpawn;
				Vec3 spawnPos = action->activeSpawnTarget->pos;

				for( int n = 0; n < toSpawn; n++ ) {
					spawnPos = PosFromSpawnZone( action->activeSpawnTarget );
					SpawnEnemy( encounter, action->ai, spawnPos, group );
				}

				//if Finsihed remove
				if( action->numSpawned == action->spawnCount ) {
					action->active = false;
					encounter->activeActions[i] = encounter->activeActions[--encounter->numActiveActions];
					printf( "End Spawn Multi AI\n" );
				}
			}break;

			default: {
				LOG_WARNING( LGS_GAME, "unsupported action type %d trying to update\n", action->type );
			}
		}
	}


	if( !encounter->block ) {
		EncounterAddActions( encounter );
		return;
	}

	if( encounter->block ) {
		switch( encounter->block->type ) {
			case ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK:
			if( gameTime > encounter->block->waitTime ) {
				encounter->block = 0;
			}break;
			case ENCOUNTER_ACTION_WAIT_FOR_SPAWN_GROUP_DEAD_BLOCK: {
				SpawnTagGroup* group = FindSpawnTagGroup( encounter, encounter->block->spawnTag );
				//Make sure group exists
				if( !group ) {
					LOG_WARNING( LGS_GAME, "Tried to Block for non existant spawn group %s\n", encounter->block->spawnTag );
					encounter->block = 0;
				}
				if( group->killed == encounter->block->spawnCount ) {
					encounter->block = 0;
				}

			}break;


			break;

		}
	}
}

void CreateEncounters() {
	// ===================== //
	// 	 First Encounter	 //
	// ===================== //
	{
		Encounter& encounter = entityManager.encounters[entityManager.numEncounters++];
		strcpy( encounter.name, "first" );

		EncounterAction& action = encounter.actions[encounter.totalActions++];
		action.type = ENCOUNTER_ACTION_SPAWN_MULTIPLE_AI;
		action.spawnCount = 4;
		action.spawnRate = 0.75f;
		action.ai = ENCOUNTER_AI_WIZARD;
		strcpy( action.spawnTag, "test" );
		strcpy( action.spawnTarget, "zone0" );

#if 0
		EncounterAction& action = encounter.actions[encounter.totalActions++];
		action.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
		action.ai = ENCOUNTER_AI_WIZARD;
		strcpy( action.spawnTarget, "spawn1" );

		{
			EncounterAction& action = encounter.actions[encounter.totalActions++];
			action.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
			action.ai = ENCOUNTER_AI_WIZARD;
			strcpy( action.spawnTarget, "zone0" );
		}
		{
			EncounterAction& action = encounter.actions[encounter.totalActions++];
			action.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
			action.ai = ENCOUNTER_AI_WIZARD;
			strcpy( action.spawnTarget, "zone0" );
		}
		{
			EncounterAction& action = encounter.actions[encounter.totalActions++];
			action.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
			action.ai = ENCOUNTER_AI_WIZARD;
			strcpy( action.spawnTarget, "zone0" );
		}
#endif
		EncounterAction& action2 = encounter.actions[encounter.totalActions++];
		action2.type = ENCOUNTER_ACTION_WAIT_FOR_SPAWN_GROUP_DEAD_BLOCK;
		action2.spawnCount = 3;
		strcpy( action2.spawnTag, "test" );

		EncounterAction& action3 = encounter.actions[encounter.totalActions++];
		action3.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
		action3.ai = ENCOUNTER_AI_GOBLIN;
		strcpy( action3.spawnTarget, "spawn2" );
	}
	
	// ===================== //
	// 	 Second Encounter	 //
	// ===================== //
	{
		Encounter& encounter = entityManager.encounters[entityManager.numEncounters++];
		strcpy( encounter.name, "second" );

		EncounterAction& action = encounter.actions[encounter.totalActions++];
		action.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
		action.ai = ENCOUNTER_AI_WIZARD;
		strcpy( action.spawnTarget, "spawn2" );

		EncounterAction& action2 = encounter.actions[encounter.totalActions++];
		action2.type = ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK;
		action2.waitTime = 3.0f;

		EncounterAction& action3 = encounter.actions[encounter.totalActions++];
		action3.type = ENCOUNTER_ACTION_SPAWN_SINGLE_AI;
		action3.ai = ENCOUNTER_AI_WIZARD;
		strcpy( action3.spawnTarget, "spawn1" );
	}
}

void TriggerTrigger( Trigger* trigger ) {
	switch( trigger->type ) {
		case TRIGGER_PRINT_MESSAGE:
		printf( "Message" );
		break;
		case TRIGGER_START_ENCOUNTER: {
			for( int i = 0; i < entityManager.numEncounters; i++ ) {
				Encounter* encounter = &entityManager.encounters[i];
				if( !strcmp( encounter->name, trigger->willTrigger ) ) {
					StartEncounter( encounter );
				}
			}
		} break;
		case TRIGGER_SPAWN_SINGLE_AI:
		{
			SpawnTarget* target = FindSpawnTarget( trigger->willTrigger );
			if( target ) {
				SpawnEnemy( 0, target->enemies, target->pos, 0 );
			}
			else
				LOG_WARNING( LGS_GAME, "Trigger can not spawn target at %s\n", trigger->willTrigger );
		}break;



		default:
		LOG_WARNING( LGS_GAME, "UNKOWN TRIGGER TYPE %d\n", trigger->type );
	}

	//Remove Trigger
	int triggerIndex = trigger - entityManager.triggers;
	entityManager.triggers[triggerIndex] = entityManager.triggers[--entityManager.numTriggers];
}