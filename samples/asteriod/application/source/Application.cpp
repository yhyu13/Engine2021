/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
Language: c++ 20
Platform: Windows 10 (X64)
Project: GAM541
Author: Hang Yu (hang.yu@digipen.edu | 60001119)
Creation date: 05/13/2020
- End Header ----------------------------*/

#include "application-precompiled-header.h"
#include "layer/Prototype2GameLayer.h"
#include "editor/layer/header/header.h"
#include "engine/Main.h"

namespace longmarch
{
	class Application final : public Engine
	{
	public:
		NONCOPYABLE(Application);
		Application()
		{
			/*
				Create the main game layer (loading game scene, game component system and game UI)
			*/
			auto gameLayer = MemoryManager::Make_shared<Prototype2GameLayer>();
			Engine::SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);
			Engine::PushLayer(gameLayer);
			/*
				Create the engine layer (engine UI, renderer pipeline)
			*/
			auto engineLayer = MemoryManager::Make_shared<_3DEditorLayer>();
			Engine::SwitchCurrentLayer(Layer::LAYER_TYPE::ENG_LAYER);
			Engine::PushLayer(engineLayer);

			// Currently, the game layer is responsible of loading all resources
			// Only after that, we call engine layer init to build all materials and meshes
			gameLayer->Init();
			engineLayer->Init();

			// Make window visible in the end of construction
			GetWindow()->SetVisible(true);

			switch (Engine::GetEngineMode())
			{
			case Engine::ENGINE_MODE::EDITING:
				Engine::GetInstance()->SwitchCurrentLayer(Layer::LAYER_TYPE::ENG_LAYER);
				break; 
			case Engine::ENGINE_MODE::INGAME:
				Engine::GetInstance()->SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);
				break;
			}
		}
	};

	Engine* CreateEngineApplication() {
		return new Application();
	}
}