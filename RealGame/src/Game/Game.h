//List of all game includes
//Should have 0 logic
#pragma once
#include "def.h"
//Entity
#include "game/Entity.h"
#include "game/Weapons.h"
#include "game/EntityManager.h"
#include "game/Player.h"

#include "game/enemies/Goblin.h"
#include "game/enemies/Ogre.h"
#include "game/enemies/Wizard.h"
#include "game/enemies/chaingunner.h"
#include "game/enemies/boar.h"

#include "game/Encounter.h"

void GameLoadEntities( const char* path );
void GameUnloadLevel();

Vec3 StringToVec3( const char* value, bool fix );
bool TryEntityField( Entity* entity, const char* key, const char* value );
void KillAI();