#include "application-precompiled-header.h"
#include "layer/MainGameLayer.h"
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
			auto gameLayer = MemoryManager::Make_shared<MainGameLayer>();
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
			
			// Switch to the intializing layer
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