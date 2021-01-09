#include "engine-precompiled-header.h"
#include "EngineEventType.h"

std::ostream& operator<<(std::ostream& o, longmarch::EngineEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineEventType::GC);
		PROCESS_VAL(longmarch::EngineEventType::SPAWN);
		PROCESS_VAL(longmarch::EngineEventType::COLLISION);

		PROCESS_VAL(longmarch::EngineEventType::ENG_WINDOW_INTERRUTPTION);
		PROCESS_VAL(longmarch::EngineEventType::ENG_WINDOW_QUIT);

	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::EngineIOEventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineIOEventType::SAVE_SCENE_BEGIN);
		PROCESS_VAL(longmarch::EngineIOEventType::SAVE_SCENE);
		PROCESS_VAL(longmarch::EngineIOEventType::SAVE_SCENE_END);
		PROCESS_VAL(longmarch::EngineIOEventType::LOAD_SCENE_BEGIN);
		PROCESS_VAL(longmarch::EngineIOEventType::LOAD_SCENE);
		PROCESS_VAL(longmarch::EngineIOEventType::LOAD_SCENE_END);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::EngineGraphicsEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SWITCH_WINDOW_MODE);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SWITCH_RESLOTION);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::TOGGLE_VSYNC);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::TOGGLE_GPUSYNC);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::TOGGLE_TAA);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::TOGGLE_FXAA);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::TOGGLE_MOTION_BLUR);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SWITCH_TONE_MAPPING);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SET_GAMMA_VALUE);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SWITCH_SHADOW_RESLOTION);
		PROCESS_VAL(longmarch::EngineGraphicsEventType::SET_AO_VALUE);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::EngineGraphicsDebugEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineGraphicsDebugEventType::SWITCH_G_BUFFER_DISPLAY);
		PROCESS_VAL(longmarch::EngineGraphicsDebugEventType::TOGGLE_SHADOW);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::EngineSettingEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::EngineSettingEventType::TOGGLE_WINDOW_INTERRUTPTION_HANDLE);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}