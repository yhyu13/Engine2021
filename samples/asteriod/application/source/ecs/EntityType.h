/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/13/2020
- End Header ----------------------------*/

#pragma once
#include <iostream>
#include <map>
#include "engine/ecs/EntityType.h"

namespace longmarch
{
	enum class GameEntityType : EntityType
	{
		EMPTY = LongMarch_ToUnderlying(EngineEntityType::NUM),
		PLAYER,
		NPC,
		

		ENEMY_ASTERIOD,
		PROJECTILE,
		SPACE_SHIP,
		NUM
	};

	struct GameEntity
	{
		inline static EntityNameToType TypeNameMap =
		{
			{"PLAYER", LongMarch_ToUnderlying(GameEntityType::PLAYER)},
			{"NPC", LongMarch_ToUnderlying(GameEntityType::NPC)},
			{"PROJECTILE", LongMarch_ToUnderlying(GameEntityType::PROJECTILE)},
			{"SPACE_SHIP", LongMarch_ToUnderlying(GameEntityType::SPACE_SHIP)},
		};
		inline static EntityTypeToName TypeNameMapInv =
		{
			{LongMarch_ToUnderlying(GameEntityType::PLAYER),"PLAYER"},
			{LongMarch_ToUnderlying(GameEntityType::NPC),"NPC"},
			{LongMarch_ToUnderlying(GameEntityType::PROJECTILE),"PROJECTILE"},
			{LongMarch_ToUnderlying(GameEntityType::SPACE_SHIP),"SPACE_SHIP"},
		};
	};
}

__LongMarch_TRVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& o, const longmarch::GameEntityType& n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::GameEntityType::PLAYER);
		PROCESS_VAL(longmarch::GameEntityType::NPC);
		PROCESS_VAL(longmarch::GameEntityType::PROJECTILE);
		PROCESS_VAL(longmarch::GameEntityType::SPACE_SHIP);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}