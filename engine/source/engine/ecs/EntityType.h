#pragma once
#include "Entity.h"
#include "engine/core/utility/TypeHelper.h"
#include <string>
#include <map>

namespace longmarch
{
	typedef LongMarch_UnorderedMap<std::string, EntityType> EntityNameToType;
	typedef LongMarch_UnorderedMap<EntityType, std::string> EntityTypeToName;

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
			{"SCENE_ROOT", LongMarch_ToUnderlying(EngineEntityType::SCENE_ROOT)},
			{"STATIC_OBJ", LongMarch_ToUnderlying(EngineEntityType::STATIC_OBJ)},
			{"DYNAMIC_OBJ", LongMarch_ToUnderlying(EngineEntityType::DYNAMIC_OBJ)},

			{"SKYSPHERE_OBJ", LongMarch_ToUnderlying(EngineEntityType::SKYSPHERE_OBJ)},
			{"SKYSBOX_OBJ", LongMarch_ToUnderlying(EngineEntityType::SKYSBOX_OBJ)},

			{"DIRECTIONAL_LIGHT", LongMarch_ToUnderlying(EngineEntityType::DIRECTIONAL_LIGHT)},
			{"POINT_LIGHT", LongMarch_ToUnderlying(EngineEntityType::POINT_LIGHT)},
			{"SPOT_LIGHT", LongMarch_ToUnderlying(EngineEntityType::SPOT_LIGHT)},

			{"EDITOR_CAMERA", LongMarch_ToUnderlying(EngineEntityType::EDITOR_CAMERA)},
			{"FREEROAM_CAMERA", LongMarch_ToUnderlying(EngineEntityType::FREEROAM_CAMERA)},
			{"PLAYER_CAMERA", LongMarch_ToUnderlying(EngineEntityType::PLAYER_CAMERA)},

			{"PLAYER_PROJECTILE", LongMarch_ToUnderlying(EngineEntityType::PLAYER_PROJECTILE)},
			{"ENEMY_PROJECTILE", LongMarch_ToUnderlying(EngineEntityType::ENEMY_PROJECTILE)},

			{"PLAYER_SHIP", LongMarch_ToUnderlying(EngineEntityType::PLAYER_SHIP)},
			{"ASTEROID", LongMarch_ToUnderlying(EngineEntityType::ASTEROID)},
			{"ENEMY_SHIP", LongMarch_ToUnderlying(EngineEntityType::ENEMY_SHIP)},
			{"METEOR", LongMarch_ToUnderlying(EngineEntityType::METEOR)},
			{"MOON_METEOR", LongMarch_ToUnderlying(EngineEntityType::MOON_METEOR)},
			{"SENTINEL_ENEMY_ROBOT", LongMarch_ToUnderlying(EngineEntityType::SENTINEL_ENEMY_ROBOT)},

			{"PLAYER_SHIP_EXHAUST", LongMarch_ToUnderlying(EngineEntityType::PLAYER_SHIP_EXHAUST)}
		};
		inline static EntityTypeToName TypeNameMapInv =
		{
			{LongMarch_ToUnderlying(EngineEntityType::SCENE_ROOT), "SCENE_ROOT"},
			{LongMarch_ToUnderlying(EngineEntityType::STATIC_OBJ), "STATIC_OBJ"},
			{LongMarch_ToUnderlying(EngineEntityType::DYNAMIC_OBJ), "DYNAMIC_OBJ"},

			{LongMarch_ToUnderlying(EngineEntityType::SKYSPHERE_OBJ), "SKYSPHERE_OBJ"},
			{LongMarch_ToUnderlying(EngineEntityType::SKYSBOX_OBJ), "SKYSBOX_OBJ"},

			{LongMarch_ToUnderlying(EngineEntityType::DIRECTIONAL_LIGHT), "DIRECTIONAL_LIGHT"},
			{LongMarch_ToUnderlying(EngineEntityType::POINT_LIGHT), "POINT_LIGHT"},
			{LongMarch_ToUnderlying(EngineEntityType::SPOT_LIGHT), "SPOT_LIGHT"},

			{LongMarch_ToUnderlying(EngineEntityType::EDITOR_CAMERA), "EDITOR_CAMERA"},
			{LongMarch_ToUnderlying(EngineEntityType::FREEROAM_CAMERA), "FREEROAM_CAMERA"},
			{LongMarch_ToUnderlying(EngineEntityType::PLAYER_CAMERA), "PLAYER_CAMERA"},

			{LongMarch_ToUnderlying(EngineEntityType::PLAYER_PROJECTILE), "PLAYER_PROJECTILE"},
			{LongMarch_ToUnderlying(EngineEntityType::ENEMY_PROJECTILE), "ENEMY_PROJECTILE"},

			{LongMarch_ToUnderlying(EngineEntityType::PLAYER_SHIP),"PLAYER_SHIP"},
			{LongMarch_ToUnderlying(EngineEntityType::ASTEROID),"ASTEROID"},
			{LongMarch_ToUnderlying(EngineEntityType::ENEMY_SHIP),"ENEMY_SHIP"},
			{LongMarch_ToUnderlying(EngineEntityType::METEOR),"METEOR"},
			{LongMarch_ToUnderlying(EngineEntityType::MOON_METEOR),"MOON_METEOR"},
			{LongMarch_ToUnderlying(EngineEntityType::SENTINEL_ENEMY_ROBOT),"SENTINEL_ENEMY_ROBOT"},

			{LongMarch_ToUnderlying(EngineEntityType::PLAYER_SHIP_EXHAUST),"PLAYER_SHIP_EXHAUST"}
		};
	};
}

__LongMarch_TRVIAL_TEMPLATE__
std::ostream& operator<<(std::ostream& o, const longmarch::EngineEntityType& n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineEntityType::EMPTY);
		PROCESS_VAL(longmarch::EngineEntityType::SCENE_ROOT);
		PROCESS_VAL(longmarch::EngineEntityType::STATIC_OBJ);
		PROCESS_VAL(longmarch::EngineEntityType::DYNAMIC_OBJ);
		PROCESS_VAL(longmarch::EngineEntityType::SKYSPHERE_OBJ);
		PROCESS_VAL(longmarch::EngineEntityType::SKYSBOX_OBJ);
		PROCESS_VAL(longmarch::EngineEntityType::DIRECTIONAL_LIGHT);
		PROCESS_VAL(longmarch::EngineEntityType::POINT_LIGHT);
		PROCESS_VAL(longmarch::EngineEntityType::SPOT_LIGHT);
		PROCESS_VAL(longmarch::EngineEntityType::EDITOR_CAMERA);

		PROCESS_VAL(longmarch::EngineEntityType::DEBUG_OBJ);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}
