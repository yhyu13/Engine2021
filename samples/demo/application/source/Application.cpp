#include "application-precompiled-header.h"
//#include "layer/MainGameLayer.h"
#include "layer/VulkanDemoLayer.h"
#ifndef APP_ONLY
//#include "editor/layer/header/header.h"
#endif
#include "engine/Main.h"

namespace longmarch
{
	class Application final : public Engine
	{
	public:
		NONCOPYABLE(Application);
		Application() = default;

		virtual void Init() override
		{
			// Call engine init first
			Engine::Init();

			// Create the engine layer (engine UI, renderer pipeline)
			auto engineLayer = MemoryManager::Make_shared<VulkanDemoLayer>();
			Engine::SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);
			Engine::PushLayer(engineLayer);
			engineLayer->Init();

//			// Create the main game layer (loading game scene, game component system and game UI)
//			auto gameLayer = MemoryManager::Make_shared<MainGameLayer>();
//			Engine::SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);
//			Engine::PushLayer(gameLayer);
//			gameLayer->Init();
//#ifndef APP_ONLY
//			// Create the engine layer (engine UI, renderer pipeline)
//			auto engineLayer = MemoryManager::Make_shared<_3DEditorLayer>();
//			Engine::SwitchCurrentLayer(Layer::LAYER_TYPE::ENG_LAYER);
//			Engine::PushLayer(engineLayer);
//			engineLayer->Init();
//#endif
			// Make window visible in the end of construction
			GetWindow()->SetVisible(true);

#ifndef APP_ONLY
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
#else
			Engine::GetInstance()->SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);
#endif
		}
	};

	Engine* CreateEngineApplication() 
	{
		return new Application();
	}
}