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

	ENCOUNTER_ACTION_WAIT_FOR_SECONDS_BLOCK,
	ENCOUNTER_ACTION_WAIT_FOR_SPAWN_GROUP_DEAD_BLOCK,
};

struct EncounterAction {
	encounterAction_t type;
	encounterEnemies_t ai;
	u16 active;

	char spawnTarget[MAX_NAME_LENGTH];
	char spawnTag[MAX_TAG_LENGTH];
	int spawnCount;
	float waitTime; 
	float spawnRate;//If spawnrate == 0 will spawn all at once //Every X Seconds one will spawn (INCLUSIVE OF 0)

	float currentTime;
	int numSpawned;

	//only valid when the spawn action has already been created
	struct SpawnTarget* activeSpawnTarget;
};

//This is so bad
struct SpawnTagGroup {
	char name[MAX_TAG_LENGTH];
	int spawned;
	int killed;
};

class Encounter {
public:
	bool active;
	char name[MAX_PATH_LENGTH];

	EncounterAction actions[32];
	//Queue of active actions, such as spawn over time
	EncounterAction* activeActions[16];
	//This is what the encounter is waiting on
	EncounterAction* block;
	//List of active spawn groups
	SpawnTagGroup spawnTags[16];

	int numActiveActions;
	int totalActions;
	int nextAction;
	int numSpawnTags;
};

void EncounterNotifyOnDeath( Encounter* encounter, class Entity* entity );
void StartEncounter( Encounter* encounter );
void UpdateEncounter( Encounter* encounter );

void ConsoleStartEncounter();
void ConsoleReloadEncounterFile();
void LoadEncounterFile( const char* path );
