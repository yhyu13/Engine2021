#pragma once
#include <iostream>

namespace longmarch
{
	enum class EngineEventType : int32_t
	{
		EMPTY = 0,
		COLLISION,
		SPAWN,
		GC,
		GC_RECURSIVE,

		ENG_WINDOW_INTERRUTPTION,
		ENG_WINDOW_QUIT,
		NUM
	};

	enum class EngineIOEventType : int32_t
	{
		EMPTY = 0,
		SAVE_SCENE_BEGIN,
		SAVE_SCENE,
		SAVE_SCENE_END,
		LOAD_SCENE_BEGIN,
		LOAD_SCENE,
		LOAD_SCENE_END,
		NUM
	};

	enum class EngineGraphicsEventType : int32_t
	{
		EMPTY = 0,
		SWITCH_WINDOW_MODE,
		SWITCH_RESLOTION,
		TOGGLE_VSYNC,
		TOGGLE_GPUSYNC,

		TOGGLE_TAA,
		TOGGLE_FXAA,
		TOGGLE_SMAA,
		TOGGLE_MOTION_BLUR,
		TOGGLE_BLOOM,
		SWITCH_TONE_MAPPING,
		SET_GAMMA_VALUE,
		SWITCH_SHADOW_RESLOTION,
		SET_AO_VALUE,
		NUM
	};

	enum class EngineGraphicsDebugEventType : int32_t
	{
		EMPTY = 0,
		SWITCH_G_BUFFER_DISPLAY,
		TOGGLE_ENV_MAPPING,
		TOGGLE_SHADOW,
		TOGGLE_DEBUG_SLICES,
		NUM
	};

	enum class EngineSettingEventType : int32_t
	{
		EMPTY = 0,
		TOGGLE_WINDOW_INTERRUTPTION_HANDLE,

		NUM
	};
}

std::ostream& operator<<(std::ostream& o, longmarch::EngineEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::EngineIOEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::EngineGraphicsEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::EngineGraphicsDebugEventType n);
std::ostream& operator<<(std::ostream& o, longmarch::EngineSettingEventType n);
