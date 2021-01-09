#pragma once
#include <iostream>

namespace longmarch
{
	enum class EditorEventType : uint32_t
	{
		EMPTY = 0,

		EDITOR_SWITCH_TO_GAME_MODE,
		EDITOR_SWITCH_TO_IN_GAME_EDITING_MODE,
		EDITOR_SWITCH_TO_EDITING_MODE,
		EDITOR_CAM_TELEPORT_TO,

		NUM
	};
}

std::ostream& operator<<(std::ostream& o, longmarch::EditorEventType n);
