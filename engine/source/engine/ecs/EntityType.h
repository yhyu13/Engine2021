#pragma once
#include "Entity.h"
#include "engine/core/utility/TypeHelper.h"
#include <string>
#include <map>

namespace AAAAgames
{
	typedef A4GAMES_UnorderedMap<std::string, EntityType> EntityNameToType;
	typedef A4GAMES_UnorderedMap<EntityType, std::string> EntityTypeToName;

	enum class EngineEntityType : EntityType
	{
		EMPTY = 0,
		SCENE_ROOT,
		STATIC_OBJ,
		DYNAMIC_OBJ,

		SKYSPHERE_OBJ,
		SKYSBOX_OBJ,

		LIGHT_OBJ_BEGIN,	// Light type order is critical
		DIRECTIONAL_LIGHT = LIGHT_OBJ_BEGIN,
		POINT_LIGHT,
		SPOT_LIGHT,
		LIGHT_OBJ_END,

		EDITOR_CAMERA,
		FREEROAM_CAMERA,
		PLAYER_CAMERA,

		DEBUG_OBJ,

		PLAYER_PROJECTILE, // = 14
		ENEMY_PROJECTILE, // = 15

		PLAYER_SHIP, // = 16
		ASTEROID,
		ENEMY_SHIP,
		METEOR,
		MOON_METEOR,
		SENTINEL_ENEMY_ROBOT, // suicide ships

		PLAYER_SHIP_EXHAUST,

		NUM
	};

	struct EngineEntity
	{
		inline static EntityNameToType TypeNameMap =
		{
			{"SCENE_ROOT", A4GAMES_ToUnderlying(EngineEntityType::SCENE_ROOT)},
			{"STATIC_OBJ", A4GAMES_ToUnderlying(EngineEntityType::STATIC_OBJ)},
			{"DYNAMIC_OBJ", A4GAMES_ToUnderlying(EngineEntityType::DYNAMIC_OBJ)},

			{"SKYSPHERE_OBJ", A4GAMES_ToUnderlying(EngineEntityType::SKYSPHERE_OBJ)},
			{"SKYSBOX_OBJ", A4GAMES_ToUnderlying(EngineEntityType::SKYSBOX_OBJ)},

			{"DIRECTIONAL_LIGHT", A4GAMES_ToUnderlying(EngineEntityType::DIRECTIONAL_LIGHT)},
			{"POINT_LIGHT", A4GAMES_ToUnderlying(EngineEntityType::POINT_LIGHT)},
			{"SPOT_LIGHT", A4GAMES_ToUnderlying(EngineEntityType::SPOT_LIGHT)},

			{"EDITOR_CAMERA", A4GAMES_ToUnderlying(EngineEntityType::EDITOR_CAMERA)},
			{"FREEROAM_CAMERA", A4GAMES_ToUnderlying(EngineEntityType::FREEROAM_CAMERA)},
			{"PLAYER_CAMERA", A4GAMES_ToUnderlying(EngineEntityType::PLAYER_CAMERA)},

			{"PLAYER_PROJECTILE", A4GAMES_ToUnderlying(EngineEntityType::PLAYER_PROJECTILE)},
			{"ENEMY_PROJECTILE", A4GAMES_ToUnderlying(EngineEntityType::ENEMY_PROJECTILE)},

			{"PLAYER_SHIP", A4GAMES_ToUnderlying(EngineEntityType::PLAYER_SHIP)},
			{"ASTEROID", A4GAMES_ToUnderlying(EngineEntityType::ASTEROID)},
			{"ENEMY_SHIP", A4GAMES_ToUnderlying(EngineEntityType::ENEMY_SHIP)},
			{"METEOR", A4GAMES_ToUnderlying(EngineEntityType::METEOR)},
			{"MOON_METEOR", A4GAMES_ToUnderlying(EngineEntityType::MOON_METEOR)},
			{"SENTINEL_ENEMY_ROBOT", A4GAMES_ToUnderlying(EngineEntityType::SENTINEL_ENEMY_ROBOT)},

			{"PLAYER_SHIP_EXHAUST", A4GAMES_ToUnderlying(EngineEntityType::PLAYER_SHIP_EXHAUST)}
		};
		inline static EntityTypeToName TypeNameMapInv =
		{
			{A4GAMES_ToUnderlying(EngineEntityType::SCENE_ROOT), "SCENE_ROOT"},
			{A4GAMES_ToUnderlying(EngineEntityType::STATIC_OBJ), "STATIC_OBJ"},
			{A4GAMES_ToUnderlying(EngineEntityType::DYNAMIC_OBJ), "DYNAMIC_OBJ"},

			{A4GAMES_ToUnderlying(EngineEntityType::SKYSPHERE_OBJ), "SKYSPHERE_OBJ"},
			{A4GAMES_ToUnderlying(EngineEntityType::SKYSBOX_OBJ), "SKYSBOX_OBJ"},

			{A4GAMES_ToUnderlying(EngineEntityType::DIRECTIONAL_LIGHT), "DIRECTIONAL_LIGHT"},
			{A4GAMES_ToUnderlying(EngineEntityType::POINT_LIGHT), "POINT_LIGHT"},
			{A4GAMES_ToUnderlying(EngineEntityType::SPOT_LIGHT), "SPOT_LIGHT"},

			{A4GAMES_ToUnderlying(EngineEntityType::EDITOR_CAMERA), "EDITOR_CAMERA"},
			{A4GAMES_ToUnderlying(EngineEntityType::FREEROAM_CAMERA), "FREEROAM_CAMERA"},
			{A4GAMES_ToUnderlying(EngineEntityType::PLAYER_CAMERA), "PLAYER_CAMERA"},

			{A4GAMES_ToUnderlying(EngineEntityType::PLAYER_PROJECTILE), "PLAYER_PROJECTILE"},
			{A4GAMES_ToUnderlying(EngineEntityType::ENEMY_PROJECTILE), "ENEMY_PROJECTILE"},

			{A4GAMES_ToUnderlying(EngineEntityType::PLAYER_SHIP),"PLAYER_SHIP"},
			{A4GAMES_ToUnderlying(EngineEntityType::ASTEROID),"ASTEROID"},
			{A4GAMES_ToUnderlying(EngineEntityType::ENEMY_SHIP),"ENEMY_SHIP"},
			{A4GAMES_ToUnderlying(EngineEntityType::METEOR),"METEOR"},
			{A4GAMES_ToUnderlying(EngineEntityType::MOON_METEOR),"MOON_METEOR"},
			{A4GAMES_ToUnderlying(EngineEntityType::SENTINEL_ENEMY_ROBOT),"SENTINEL_ENEMY_ROBOT"},

			{A4GAMES_ToUnderlying(EngineEntityType::PLAYER_SHIP_EXHAUST),"PLAYER_SHIP_EXHAUST"}
		};
	};
}

__A4GAMES_TRVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& o, const AAAAgames::EngineEntityType& n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineEntityType::EMPTY);
		PROCESS_VAL(AAAAgames::EngineEntityType::SCENE_ROOT);
		PROCESS_VAL(AAAAgames::EngineEntityType::STATIC_OBJ);
		PROCESS_VAL(AAAAgames::EngineEntityType::DYNAMIC_OBJ);
		PROCESS_VAL(AAAAgames::EngineEntityType::SKYSPHERE_OBJ);
		PROCESS_VAL(AAAAgames::EngineEntityType::SKYSBOX_OBJ);
		PROCESS_VAL(AAAAgames::EngineEntityType::DIRECTIONAL_LIGHT);
		PROCESS_VAL(AAAAgames::EngineEntityType::POINT_LIGHT);
		PROCESS_VAL(AAAAgames::EngineEntityType::SPOT_LIGHT);
		PROCESS_VAL(AAAAgames::EngineEntityType::EDITOR_CAMERA);

		PROCESS_VAL(AAAAgames::EngineEntityType::DEBUG_OBJ);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}
