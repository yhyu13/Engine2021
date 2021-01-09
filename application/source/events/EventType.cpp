#include "application-precompiled-header.h"
#include "EventType.h"

std::ostream& operator<<(std::ostream& o, longmarch::Prototype2EventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::Prototype2EventType::GEN_PROJECTILE);
		PROCESS_VAL(longmarch::Prototype2EventType::GEN_NPC_ASTERIOD);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::CS560EventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::CS560EventType::GEN_RND_PATH);
		PROCESS_VAL(longmarch::CS560EventType::SET_PATHER_MISC);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::GameEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(longmarch::GameEventType::EMPTY);
		PROCESS_VAL(longmarch::GameEventType::SOUND);
		PROCESS_VAL(longmarch::GameEventType::DEATH);
		PROCESS_VAL(longmarch::GameEventType::SPAWN);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::GameDebugEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, longmarch::GameSettingEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}