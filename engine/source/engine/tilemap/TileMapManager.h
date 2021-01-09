#pragma once
#include "TileMap.h"

namespace longmarch {
	/*
		Use example
		TileMapManager::GetInstance()->Init;
		TileMapManager::GetInstance()->Add("Level1");
		TileMapManager::GetInstance()->SetCurrentMapName("Level1");
		// create a derived class of TileMapManager to creat your custom tilemap loader and unloader
		...
		TileMapManager::GetInstance()->ShutDown();
	*/
	class TileMapManager
	{
	public:
		static TileMapManager* GetInstance()
		{
			static TileMapManager instance;
			return &instance;
		}

		void SetCurrentMapName(const std::string& name);
		const std::string& GetCurrentMapName();
		std::shared_ptr<TileMap> GetCurrentMap();
		void AddTileMap(const std::string& name);

	protected:
		std::string m_currentMapName;
		std::unordered_map<std::string, std::shared_ptr<TileMap>> m_tileMaps;
	};
}