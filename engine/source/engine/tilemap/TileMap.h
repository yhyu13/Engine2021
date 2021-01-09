#pragma once
#include "engine/EngineEssential.h"
#include "engine/math/Geommath.h"
#include "engine/math/GridF32.h"
#include "engine/ai/PathFinding/PathFinder2DGrid.h"
#include <Tileson.h>

namespace longmarch {
	extern float LongMarch_GetPixel2WorldNumerator();

	/*
	Use case:
		auto tileMapObj = GameLevelMapManager::GetInstance()->GetCurrentMap();
		auto pathGrid = tileMapObj->GetTileGrid("Path");
		auto Astar = tileMapObj->GetPathFinder("Path");
		if (Astar->Search(*pathGrid, _src, _dest))
		{
			auto result = Astar->GetResult();
		}
		else
		{
			...
		}
	*/
	class TileMap
	{
	public:
		typedef GridF32 TileGrid;
		typedef pathfinding::PathFinder2DGrid<int16_t, float> PathFinder;

		explicit TileMap(const std::string& name)
			:
			m_name(name),
			m_Map(nullptr),
			m_PathFinder(nullptr)
		{}
		static std::shared_ptr<TileMap> LoadFromFile(const fs::path& path);
		std::shared_ptr<tson::Map> GetMap();
		TileGrid* GetTileGrid(const std::string& name);
		std::shared_ptr<PathFinder> GetPathFinder(const std::string& name);
		/*
			World position to tile grid position
		*/
		Vec2i World2Grid(const Vec2f& v);
		/*
			World position to tile pixel position
		*/
		Vec2f World2Pixel(const Vec2f& v);
		/*
			Tile pixel position to world position
		*/
		Vec2f Pixel2World(const Vec2f& v);
		/*
			Tile grid position to world position
		*/
		Vec2f Grid2World(const Vec2i& v);

	private:
		std::string m_name;
		std::shared_ptr<tson::Map> m_Map;
		std::shared_ptr<PathFinder> m_PathFinder;
		std::unordered_map<std::string, TileGrid> m_Grids;
	};
}
