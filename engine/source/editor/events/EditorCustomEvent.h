#pragma once

#include "EditorEventType.h"
#include "engine/events/EventQueue.h"
#include "engine/ecs/EntityDecorator.h"

namespace AAAAgames
{
	struct EditorCameraTeleportToEntityEvent : public Event<EditorEventType> {
		explicit EditorCameraTeleportToEntityEvent(const EntityDecorator& entity)
			:
			Event(EditorEventType::EDITOR_CAM_TELEPORT_TO),
			m_entity(entity)
		{
		}
		EntityDecorator m_entity;
	};

	struct EditorSwitchToGameModeEvent : public Event<EditorEventType> {
		explicit EditorSwitchToGameModeEvent(void* world)
			:
			Event(EditorEventType::EDITOR_SWITCH_TO_GAME_MODE),
			editing_world(world)
		{
		}
		void* editing_world;
	};

	struct EditorSwitchToEditingModeEvent : public Event<EditorEventType> {
		explicit EditorSwitchToEditingModeEvent(void* world)
			:
			Event(EditorEventType::EDITOR_SWITCH_TO_EDITING_MODE),
			editing_world(world)
		{
		}
		void* editing_world;
	};
}