#pragma once
#include <iostream>
#include <map>
#include "engine/ecs/EntityType.h"

namespace AAAAgames
{
	enum class GameEntityType : EntityType
	{
		EMPTY = A4GAMES_ToUnderlying(EngineEntityType::NUM),
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
			{"PLAYER", A4GAMES_ToUnderlying(GameEntityType::PLAYER)},
			{"NPC", A4GAMES_ToUnderlying(GameEntityType::NPC)},
			{"PROJECTILE", A4GAMES_ToUnderlying(GameEntityType::PROJECTILE)},
			{"SPACE_SHIP", A4GAMES_ToUnderlying(GameEntityType::SPACE_SHIP)},
		};
		inline static EntityTypeToName TypeNameMapInv =
		{
			{A4GAMES_ToUnderlying(GameEntityType::PLAYER),"PLAYER"},
			{A4GAMES_ToUnderlying(GameEntityType::NPC),"NPC"},
			{A4GAMES_ToUnderlying(GameEntityType::PROJECTILE),"PROJECTILE"},
			{A4GAMES_ToUnderlying(GameEntityType::SPACE_SHIP),"SPACE_SHIP"},
		};
	};
}

__A4GAMES_TRVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& o, const AAAAgames::GameEntityType& n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::GameEntityType::PLAYER);
		PROCESS_VAL(AAAAgames::GameEntityType::NPC);
		PROCESS_VAL(AAAAgames::GameEntityType::PROJECTILE);
		PROCESS_VAL(AAAAgames::GameEntityType::SPACE_SHIP);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}