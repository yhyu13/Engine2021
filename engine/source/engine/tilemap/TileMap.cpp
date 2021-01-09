#include "engine-precompiled-header.h"
#include "TileMap.h"
#include "engine/renderer/Texture.h"

namespace fs = std::filesystem;
/*
	Implementation reference : https://github.com/SSBMTonberry/tileson
*/
std::shared_ptr<TileMap> AAAAgames::TileMap::LoadFromFile(const fs::path& path)
{
	tson::Tileson parser;
	auto result = MemoryManager::Make_shared<TileMap>(path.filename().string());

	// Hang Yu comment:
	// I encountered a strange bug where using MemoryManager:Make_shared to create the tson::Map
	// would cause a faliure in copy construction. The effect was a randomness of loss of data because
	// the compiler's creation of the copy constructor of tson::Map is not complete.
	// Using std::make_shared solves this problem.
	// The strange part is that it happens in release mode where MemoryManager:Make_shared simply wraps std::make_shared
	result->m_Map = std::make_shared<tson::Map>(parser.parse(path));
	auto map = result->m_Map;

	if (map->getStatus() == tson::Map::ParseStatus::OK)
	{
		auto resourceManager = ResourceManager<Texture2D>::GetInstance();
		resourceManager->SetLoadFromFileFunc(Texture2D::LoadFromFile);
		for (auto& tileset : map->getTilesets())
		{
			auto path = tileset.getImagePath().string();
			DEBUG_PRINT("Loading tileset at " + path);
			resourceManager->LoadFromFile(path, tileset.getName());
		}
		return result;
	}
	else if (map->getStatus() == tson::Map::ParseStatus::FileNotFound)
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"TJson tilemap at " + str2wstr(path.string()) + L" does not exist!");
	}
	else if (map->getStatus() == tson::Map::ParseStatus::MissingData)
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Json tilemap at " + str2wstr(path.string()) + L" has missing data!");
	}
	else if (map->getStatus() == tson::Map::ParseStatus::ParseError)
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Tileson fails to parse json tilemap at " + str2wstr(path.string()) + L"!");
	}

	return nullptr;
}

std::shared_ptr<tson::Map> AAAAgames::TileMap::GetMap()
{
	return m_Map;
}

AAAAgames::TileMap::TileGrid* AAAAgames::TileMap::GetTileGrid(const std::string& name)
{
	if (m_Grids.find(name) == m_Grids.end())
	{
		m_Grids[name] = TileGrid(m_Map->getSize().x, m_Map->getSize().y);
	}
	return &m_Grids[name];
}

std::shared_ptr<AAAAgames::TileMap::PathFinder> AAAAgames::TileMap::GetPathFinder(const std::string& name)
{
	if (!m_PathFinder)
	{
		auto pathGrid = GetTileGrid(name);
		m_PathFinder = MemoryManager::Make_shared<PathFinder>();
		m_PathFinder->Init(pathGrid->X(), pathGrid->Y());
	}
	return m_PathFinder;
}

Vec2i AAAAgames::TileMap::World2Grid(const Vec2f& v)
{
	return Vec2i(int(v.x * A4GAMES_GetPixel2WorldNumerator() / m_Map->getTileSize().x),
		int(-v.y * A4GAMES_GetPixel2WorldNumerator() / m_Map->getTileSize().y));
}

Vec2f AAAAgames::TileMap::World2Pixel(const Vec2f& v)
{
	return Vec2f(v.x * A4GAMES_GetPixel2WorldNumerator(),
		-v.y * A4GAMES_GetPixel2WorldNumerator());
}

Vec2f AAAAgames::TileMap::Pixel2World(const Vec2f& v)
{
	return Vec2f((float)v.x / A4GAMES_GetPixel2WorldNumerator(),
		(float)-v.y / A4GAMES_GetPixel2WorldNumerator());
}

Vec2f AAAAgames::TileMap::Grid2World(const Vec2i& v)
{
	return Vec2f((float)v.x * (float)m_Map->getTileSize().x / A4GAMES_GetPixel2WorldNumerator(),
		(float)-v.y * (float)m_Map->getTileSize().y / A4GAMES_GetPixel2WorldNumerator());
}