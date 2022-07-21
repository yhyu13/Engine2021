#include "engine-precompiled-header.h"
#include "TileMapManager.h"

void longmarch::TileMapManager::SetCurrentMapName(const std::string& name)
{
	if (m_tileMaps.find(name) != m_tileMaps.end())
	{
		m_currentMapName = name;
	}
	else
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"The map " + wStr(name) + L" has not been managed!");
	}
}

const std::string& longmarch::TileMapManager::GetCurrentMapName()
{
	return m_currentMapName;
}

std::shared_ptr<TileMap> longmarch::TileMapManager::GetCurrentMap()
{
	auto it = m_tileMaps.find(m_currentMapName);
	if (it != m_tileMaps.end())
	{
		return it->second;
	}
	else
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"The map " + wStr(m_currentMapName) + L" has not been added!");
	}
}

void longmarch::TileMapManager::AddTileMap(const std::string& name)
{
	m_tileMaps[name] = ResourceManager<TileMap>::GetInstance()->TryGet(name)->Get();
}
