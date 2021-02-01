#pragma once
#include <iostream>
#include <vector>

namespace longmarch
{
	enum class GameEventType : int32_t
	{
		EMPTY = 0,
		SOUND,									// Play sound event
		SOUND_AT_PLAYER,
		SOUND_AT_CAMERA,
		MUTE_SFX,                             // Mute Sound Event
		MUTE_BGM,                             // Mute music Event
		DEATH,									// Death event
		SPAWN,									// Spawning event

		NUM
	};

	enum class CS560EventType : int32_t
	{
		EMPTY = 0,
		GEN_RND_PATH,
		SET_PATHER_CONTROL,
		SET_PATHER_MISC,
		SET_PATHER_SMOOTH_SETTING,
		NUM
	};

	enum class Prototype2EventType : int32_t
	{
		EMPTY = 0,
		GEN_PROJECTILE,
		GEN_SPACE_SHIP,
		GEN_NPC_ASTERIOD,
		NUM
	};

	enum class GameDebugEventType : int32_t
	{
		EMPTY = 0,

		NUM
	};

	enum class GameSettingEventType : int32_t
	{
		EMPTY = 0,
		NUM
	};
}


std::ostream& operator<<(std::ostream& o, longmarch::Prototype2EventType n);
std::ostream& operator<<(std::ostream& o, longmarch::CS560EventType n);
std::ostream& operator<<(std::ostream& o, longmarch::GameEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::GameDebugEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::GameSettingEventType n);
