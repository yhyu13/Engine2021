#include "engine-precompiled-header.h"
#include "BaseEngineWidgetManager.h"
#include "engine/ecs/GameWorld.h"

void AAAAgames::BaseEngineWidgetManager::BeginFrame()
{
	UpdateSelectedEntity();
}

void AAAAgames::BaseEngineWidgetManager::EndFrame()
{
	CaptureMouseAndKeyboardOnMenu();
	UpdateGameWorldTabs();
}

void AAAAgames::BaseEngineWidgetManager::CaptureMouseAndKeyboardOnMenu()
{
	ImGuiIO& io = ImGui::GetIO();
	bool isWindowFocused = ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered();
	// Set ImGui capture inputs on window focused
	if (isWindowFocused)
	{
		io.WantCaptureMouse = isWindowFocused;
		io.WantCaptureKeyboard = isWindowFocused;
	}
}

void AAAAgames::BaseEngineWidgetManager::PushBackSelectedEntity(const Entity& e)
{
	if (!A4GAMES_Contains(m_SelectedEntityBuffer, e))
	{
		m_SelectedEntityBuffer.emplace_back(e);
	}
}

const A4GAMES_Vector<Entity> AAAAgames::BaseEngineWidgetManager::GetAllSelectedEntity()
{
	return m_SelectedEntity;
}

const A4GAMES_Vector<Entity> AAAAgames::BaseEngineWidgetManager::GetAllSelectedEntityBuffered()
{
	return m_SelectedEntityBuffer;
}

void AAAAgames::BaseEngineWidgetManager::UpdateSelectedEntity()
{
	m_SelectedEntity = m_SelectedEntityBuffer;
	m_SelectedEntityBuffer.clear();
}

void AAAAgames::BaseEngineWidgetManager::AddNewGameWorldLevel(const std::string& name)
{
	for (auto& [_, isSelect, __, ___] : m_gameWorldLevels)
	{
		isSelect = false;
	}
	// Erase previous the same existed game world and push it to be very back
	std::erase_if(m_gameWorldLevels, [&](const auto& item)->bool { return std::get<0>(item) == name; });
	m_gameWorldLevels.emplace_back(name, true, true, false);
}

void AAAAgames::BaseEngineWidgetManager::UpdateGameWorldTabs()
{
	// Remove closed game worlds
	{
		for (auto& [name, isSelect, isVisible, shouldRemove] : m_gameWorldLevels)
		{
			if (shouldRemove)
			{
				GameWorld::RemoveManagedWorld(name);
				isSelect = isVisible = shouldRemove = false;
			}
		}
	}
	// Make all managed game world visible
	{
		static auto _contains = [](auto& levels, const std::string& name)
		{
			for (auto it = levels.begin(); it != levels.end(); ++it)
			{
				if (std::get<0>(*it) == name)
				{
					return it;
				}
			}
			return levels.end();
		};
		for (auto& name : GameWorld::GetAllManagedWorldNames())
		{
			if (auto it = _contains(m_gameWorldLevels, name); it != m_gameWorldLevels.end())
			{
				std::get<2>(*it) = true; // Making all managed levels to have visible tabs
				std::get<3>(*it) = false;
			}
			else
			{
				AddNewGameWorldLevel(name); // Add new levels
			}
		}
	}
	// Switch to selected game world
	{
		bool has_selected = false;
		for (auto& [name, isSelect, isVisible, _] : m_gameWorldLevels)
		{
			// At most one selection, set all other game levels not be non-selected
			if (has_selected)
			{
				isSelect = false;
			}
			if (isSelect)
			{
				if (isVisible)
				{
					has_selected = true;
					auto world = GameWorld::GetManagedWorldByName(name);
					ASSERT(world, "Gameworld is nullptr!");
					// current world is not selected world, switch to selected world
					if (GameWorld::GetCurrent() != world)
					{
						PRINT("Switch game world to " + name);
						GameWorld::SetCurrent(world);
					}
					continue;
				}
				else
				{
					break;
				}
			}
		}
		// If none is selected, select the first visible game world
		if (!has_selected)
		{
			for (auto& [name, isSelect, isVisible, _] : m_gameWorldLevels)
			{
				if (isVisible)
				{
					has_selected = true;
					isSelect = true;
					auto world = GameWorld::GetManagedWorldByName(name);
					ASSERT(world, "Gameworld is nullptr!");
					// current world is not selected world, switch to selected world
					if (GameWorld::GetCurrent() != world)
					{
						PRINT("Switch game world to " + name);
						GameWorld::SetCurrent(world);
					}
					break;
				}
			}
		}
		ENGINE_EXCEPT_IF(!has_selected, L"There is no valid game world for selection!");
	}
}