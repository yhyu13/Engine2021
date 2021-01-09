#include "engine-precompiled-header.h"
#include "EditorEventType.h"

std::ostream& operator<<(std::ostream& o, AAAAgames::EditorEventType n)
{
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (n) {
		PROCESS_VAL(AAAAgames::EditorEventType::EDITOR_SWITCH_TO_GAME_MODE);
		PROCESS_VAL(AAAAgames::EditorEventType::EDITOR_SWITCH_TO_IN_GAME_EDITING_MODE);
		PROCESS_VAL(AAAAgames::EditorEventType::EDITOR_SWITCH_TO_EDITING_MODE);
		PROCESS_VAL(AAAAgames::EditorEventType::EDITOR_CAM_TELEPORT_TO);
	default: s = "(invalid value)"; break;
	}
#undef PROCESS_VAL
	return o << s;
}