#include "engine-precompiled-header.h"
#include "BaseEngineWidgetManager.h"
#include "engine/ecs/GameWorld.h"

void longmarch::BaseEngineWidgetManager::BeginFrame()
{
    BaseWidgetManager::BeginFrame();
    UpdateSelectedEntity();
}

void longmarch::BaseEngineWidgetManager::EndFrame()
{
    UpdateGameWorldTabs();
    BaseWidgetManager::EndFrame();
}

void longmarch::BaseEngineWidgetManager::PushBackSelectedEntityBuffered(const Entity& e)
{
    if (!LongMarch_Contains(m_SelectedEntityBuffer, e))
    {
        m_SelectedEntityBuffer.emplace_back(e);
    }
}

void longmarch::BaseEngineWidgetManager::EraseSelectedEntityBuffered(const Entity& e)
{
    if (int index = LongMarch_findFristIndex(m_SelectedEntityBuffer, e); index != -1)
    {
        m_SelectedEntityBuffer.erase(m_SelectedEntityBuffer.begin() + index);
    }
}

const LongMarch_Vector<Entity> longmarch::BaseEngineWidgetManager::GetAllSelectedEntity()
{
    return m_SelectedEntity;
}

const LongMarch_Vector<Entity> longmarch::BaseEngineWidgetManager::GetAllSelectedEntityBuffered()
{
    return m_SelectedEntityBuffer;
}

void longmarch::BaseEngineWidgetManager::UpdateSelectedEntity()
{
    m_SelectedEntity = m_SelectedEntityBuffer;
    m_SelectedEntityBuffer.clear();
}

void longmarch::BaseEngineWidgetManager::AddNewGameWorldLevel(const std::string& name)
{
    for (auto& [_, isSelect, __, ___] : m_gameWorldLevels)
    {
        isSelect = false;
    }
    // Erase previous the same existed game world and push it to be very back
    std::erase_if(m_gameWorldLevels, [&](const auto& item)-> bool { return std::get<0>(item) == name; });
    m_gameWorldLevels.emplace_back(name, true, true, false);
}

void longmarch::BaseEngineWidgetManager::UpdateGameWorldTabs()
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
