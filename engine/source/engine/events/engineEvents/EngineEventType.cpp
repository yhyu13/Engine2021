#include "engine-precompiled-header.h"
#include "EngineEventType.h"

std::ostream& operator<<(std::ostream& o, AAAAgames::EngineEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineEventType::GC);
		PROCESS_VAL(AAAAgames::EngineEventType::SPAWN);
		PROCESS_VAL(AAAAgames::EngineEventType::COLLISION);

		PROCESS_VAL(AAAAgames::EngineEventType::ENG_WINDOW_INTERRUTPTION);
		PROCESS_VAL(AAAAgames::EngineEventType::ENG_WINDOW_QUIT);

	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::EngineIOEventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineIOEventType::SAVE_SCENE_BEGIN);
		PROCESS_VAL(AAAAgames::EngineIOEventType::SAVE_SCENE);
		PROCESS_VAL(AAAAgames::EngineIOEventType::SAVE_SCENE_END);
		PROCESS_VAL(AAAAgames::EngineIOEventType::LOAD_SCENE_BEGIN);
		PROCESS_VAL(AAAAgames::EngineIOEventType::LOAD_SCENE);
		PROCESS_VAL(AAAAgames::EngineIOEventType::LOAD_SCENE_END);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::EngineGraphicsEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SWITCH_WINDOW_MODE);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SWITCH_RESLOTION);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::TOGGLE_VSYNC);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::TOGGLE_GPUSYNC);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::TOGGLE_TAA);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::TOGGLE_FXAA);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::TOGGLE_MOTION_BLUR);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SWITCH_TONE_MAPPING);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SET_GAMMA_VALUE);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SWITCH_SHADOW_RESLOTION);
		PROCESS_VAL(AAAAgames::EngineGraphicsEventType::SET_AO_VALUE);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::EngineGraphicsDebugEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineGraphicsDebugEventType::SWITCH_G_BUFFER_DISPLAY);
		PROCESS_VAL(AAAAgames::EngineGraphicsDebugEventType::TOGGLE_SHADOW);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::EngineSettingEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EngineSettingEventType::TOGGLE_WINDOW_INTERRUTPTION_HANDLE);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}