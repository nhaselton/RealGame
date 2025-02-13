#include "encounter.h"
#include "Game.h"
#include "EntityManager.h"

void SpawnEnemy( encounterEnemies_t enemy, Vec3 pos ) {
	Entity* e = 0; 
	switch( enemy ) {
		case ENCOUNTER_AI_GOBLIN: e = CreateGoblin( pos ); break;
		case ENCOUNTER_AI_WIZARD: e = CreateWizard( pos ); break;
		default:
		LOG_WARNING( LGS_GAME, "Trying to spawn Enemy type %d that does not exist\n", enemy );
	}
}

void EncounterAddActions( Encounter* encounter ) {
	for( int i = encounter->nextAction; i < encounter->totalActions; i++, encounter->nextAction++ ) {
		if( encounter->block )
			return;

		EncounterAction* action = &encounter->actions[i];
		switch( action->type ) {
			case ENCOUNTER_ACTION_SPAWN_SINGLE_AI: {
				printf( "Encounter Spawn AI\n" );
				for( int n = 0; n < entityManager.numSpawnTargets; n++ ) {
					SpawnTarget* target = &entityManager.spawnTargets[n];
					if( !strcmp( target->name, action->spawnTarget ) ) {
						SpawnEnemy( action->ai, target->pos );
					}
				}
			} break;
			case ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK:
			{
				printf( "Encounter Wait For Seconds\n" );
				encounter->block = action;
				action->waitTime += gameTime;
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

	if( !encounter->block ) {
		EncounterAddActions( encounter );
		return;
	}

	if( encounter->block ) {
		switch( encounter->block->type ) {
			case ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK:
			if( gameTime > encounter->block->waitTime ) {
				encounter->block = 0;
			}
			break;
		}
	}

}
