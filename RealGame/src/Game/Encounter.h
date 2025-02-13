#pragma once
#include "def.h"
enum encounterEnemies_t : u16 {
	ENCOUNTER_AI_NONE = 0,
	ENCOUNTER_AI_GOBLIN = 1,
	ENCOUNTER_AI_WIZARD = 2,
};



enum encounterAction_t : u16 {
	ENCOUNTER_ACTION_SPAWN_SINGLE_AI = 1,
	ENCOUNTER_ACTION_SPAWN_MULTIPLE_AI = 2,
	ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK = 3,
	ENCOUNTER_ACTION_WAIT_FOR_DEAD_BLOCK = 4,
};

struct EncounterAction {
	encounterAction_t type;
	encounterEnemies_t ai;
	u16 active;

	char spawnTarget[MAX_NAME_LENGTH];
	int spawnCount;
	float waitTime;
};

class Encounter {
public:
	bool active;
	char name[MAX_PATH_LENGTH];

	EncounterAction actions[128];
	//Queue of active actions, such as spawn over time
	EncounterAction* activeActions[16];
	//This is what the encounter is waiting on
	EncounterAction* block;
	int numActiveActions;
	int totalActions;
	int nextAction;
};

//Spawn Single AI 
	//spawner
	//Spawn Zone

void StartEncounter( Encounter* encounter );
void UpdateEncounter( Encounter* encounter );
void CompleteEncounter( Encounter* encounter );

//These are hardcoded in engine right now
void CreateEncounters();