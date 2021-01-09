#include "application-precompiled-header.h"
#include "EventType.h"

std::ostream& operator<<(std::ostream& o, AAAAgames::Prototype2EventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::Prototype2EventType::GEN_PROJECTILE);
		PROCESS_VAL(AAAAgames::Prototype2EventType::GEN_NPC_ASTERIOD);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::CS560EventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::CS560EventType::GEN_RND_PATH);
		PROCESS_VAL(AAAAgames::CS560EventType::SET_PATHER_MISC);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::GameEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::GameEventType::EMPTY);
		PROCESS_VAL(AAAAgames::GameEventType::SOUND);
		PROCESS_VAL(AAAAgames::GameEventType::DEATH);
		PROCESS_VAL(AAAAgames::GameEventType::SPAWN);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::GameDebugEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}

std::ostream& operator<<(std::ostream& o, AAAAgames::GameSettingEventType n) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}