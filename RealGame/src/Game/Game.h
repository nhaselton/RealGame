//List of all game includes
//Should have 0 logic
#pragma once
//Entity
#include "Entity.h"
#include "Weapons.h"
#include "EntityManager.h"
#include "Player.h"

#include "Goblin.h"
#include "Ogre.h"
#include "Wizard.h"

#include "Encounter.h"

void GameLoadEntities( const char* path );
void GameUnloadLevel();

Vec3 StringToVec3( const char* value, bool fix );
bool TryEntityField( Entity* entity, const char* key, const char* value );
void KillAI();