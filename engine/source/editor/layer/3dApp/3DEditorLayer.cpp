#include "engine-precompiled-header.h"
#include "3DEditorLayer.h"
#include "engine/ecs/header/header.h"
#include "editor/ui/header/header.h"
#include "editor/events/EditorCustomEvent.h"

longmarch::_3DEditorLayer::_3DEditorLayer()
	: Layer("_3DEditorLayer")
{
	Renderer3D::Init();
}

void longmarch::_3DEditorLayer::Init()
{
	BuildRenderPipeline();
	{
		// Register _3DEngineWidgetManager
		ServiceLocator::ProvideSingleton<_3DEngineWidgetManager>(ENG_WIG_MAN_NAME, MemoryManager::Make_shared<_3DEngineWidgetManager>());
	}
	{
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		m_font = io.Fonts->AddFontFromFileTTF(FileSystem::ResolveProtocol("$asset:ImGui/Roboto-Regular.ttf").string().c_str(), 18.0f);
	}
	{
		auto queue = EventQueue<EditorEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<_3DEditorLayer>(this, EditorEventType::EDITOR_SWITCH_TO_EDITING_MODE, &_3DEditorLayer::_ON_EDITOR_SWITCH_TO_EDITING_MODE));
		ManageEventSubHandle(queue->Subscribe<_3DEditorLayer>(this, EditorEventType::EDITOR_SWITCH_TO_GAME_MODE, &_3DEditorLayer::_ON_EDITOR_SWITCH_TO_GAME_MODE));
	
		Engine::GetInstance()->Update().Connect([](double dt) 
		{
			{
				auto queue = EventQueue<EditorEventType>::GetInstance();
				queue->Update(dt);
			}
		});
	}
}

void longmarch::_3DEditorLayer::OnUpdate(double ts)
{
	double dt = GameWorld::GetCurrent()->IsPaused() ? 0.0 : ts;
	{
		{
			ENG_TIME("Pre System update");
			PreUpdate(dt);
		}
		{
			ENG_TIME("PreRender update");
			PreRenderUpdate(dt);
		}
		{
			ENG_TIME("System update");
			Update(dt);
		}
		{
			ENG_TIME("Render");
			Render(dt);
		}
		{
			JoinAll();
		}
		{
			ENG_TIME("PostRender update");
			PostRenderUpdate(dt);
		}
	}
}

void longmarch::_3DEditorLayer::OnAttach()
{
	PRINT("_3DEditorLayer attached!");
}

void longmarch::_3DEditorLayer::OnDetach()
{
	PRINT("_3DEditorLayer detached!");
}

void longmarch::_3DEditorLayer::OnImGuiRender()
{
	ENG_TIME("Engine ImGUI Render");
	ImGui::PushFont(m_font);
	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->BeginFrame();
	manager->RenderUI();
	manager->EndFrame();
	GameWorld::GetCurrent()->RenderUI();
	ImGui::PopFont();
}

void longmarch::_3DEditorLayer::PreUpdate(double ts)
{
}

void longmarch::_3DEditorLayer::Update(double ts)
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadUpdate(ts);
#else
	GameWorld::GetCurrent()->Update(ts);
#endif // MULTITHREAD_UPDATE
}

void longmarch::_3DEditorLayer::JoinAll()
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadJoin();
#endif // MULTITHREAD_UPDATE
}

void longmarch::_3DEditorLayer::PreRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PreRenderUpdate(ts);
}

void longmarch::_3DEditorLayer::Render(double ts)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::EDITING:
	{
		GPU_TIME(Total_Render);
		GameWorld::GetCurrent()->Render(ts);
		m_Data.mainRenderPipeline(ts);
		GameWorld::GetCurrent()->Render2(ts);
		Renderer3D::SubmitFrameBufferToScreen();
	}
		break;
	default:
		return;
	}
}

void longmarch::_3DEditorLayer::PostRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PostRenderUpdate(ts);
}

void longmarch::_3DEditorLayer::BuildRenderPipeline()
{
	m_Data.mainRenderPipeline = [this](double ts)
	{
		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
			e_type = (EntityType)EngineEntityType::EDITOR_CAMERA;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		// TODO move render pipeline for INGame to application's layer, and use application layer in application mode
		auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
		auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::EDITING:
			// Camera viewport rect is set by scene dock widget every frame (see SceneDock.cpp)
			break;
		break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		if (Renderer3D::ShouldRendering())
		{
			// callbacks for scene rendering
			auto scene3DComSys = static_cast<Scene3DComSys*>(GameWorld::GetCurrent()->GetComponentSystem("Scene3DComSys"));
			std::function<void()> f_render_opaque = std::bind(&Scene3DComSys::RenderOpaqueObj, scene3DComSys);
			std::function<void()> f_render_translucent = std::bind(&Scene3DComSys::RenderTransparentObj, scene3DComSys);
			std::function<void(bool, const ViewFrustum&, const Mat4&)> f_setVFCullingParam = std::bind(&Scene3DComSys::SetVFCullingParam, scene3DComSys, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			std::function<void(bool, const Vec3f&, float, float)> f_setDistanceCullingParam = std::bind(&Scene3DComSys::SetDistanceCullingParam, scene3DComSys, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			std::function<void(const std::string&)> f_setRenderShaderName = std::bind(&Scene3DComSys::SetRenderShaderName, scene3DComSys, std::placeholders::_1);

			{
				Renderer3D::BeginRendering(cam);
				{
					GPU_TIME(Shadow_Pass);
					ENG_TIME("Shadow pass");
					Renderer3D::BeginShadowing(cam, f_render_opaque, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndShadowing();
				}
				{
					GPU_TIME(Opaque_Scene_pass);
					ENG_TIME("Opaque Scene pass");
					Renderer3D::BeginOpaqueScene(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueScene();
				}
				{
					GPU_TIME(Opaque_Lighting_pass);
					ENG_TIME("Opaque Lighting pass");
					Renderer3D::BeginOpaqueLighting(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueLighting();
				}
				{
					GPU_TIME(Transparent_Scene_pass);
					ENG_TIME("Transparent Scene pass");
					Renderer3D::BeginTransparentSceneAndLighting(cam, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndTransparentSceneAndLighting();
				}
				{
					GPU_TIME(Postprocessing_pass);
					ENG_TIME("Postprocessing pass");
					Renderer3D::BeginPostProcessing();
					Renderer3D::EndPostProcessing();
				}
				Renderer3D::EndRendering();
			}
		}
	};
}


void longmarch::_3DEditorLayer::_ON_EDITOR_SWITCH_TO_GAME_MODE(EventQueue<EditorEventType>::EventPtr e)
{
	if (auto event = std::dynamic_pointer_cast<EditorSwitchToGameModeEvent>(e); event)
	{
		Engine::SetEngineMode(Engine::ENGINE_MODE::INGAME);
		Engine::GetInstance()->SwitchCurrentLayer(Layer::LAYER_TYPE::APP_LAYER);

		auto world = static_cast<GameWorld*>(event->editing_world);
		GameWorld::SetCurrent(GameWorld::Clone(world->GetName() + "_gameMode", world));
	}
	else
	{
		ENGINE_EXCEPT(L"Event casting failed!");
	}
}

void longmarch::_3DEditorLayer::_ON_EDITOR_SWITCH_TO_EDITING_MODE(EventQueue<EditorEventType>::EventPtr e)
{
	if (auto event = std::dynamic_pointer_cast<EditorSwitchToEditingModeEvent>(e); event)
	{
		Engine::SetEngineMode(Engine::ENGINE_MODE::EDITING);
		Engine::GetInstance()->SwitchCurrentLayer(Layer::LAYER_TYPE::ENG_LAYER);

		auto world = static_cast<GameWorld*>(event->editing_world);
		GameWorld::RemoveManagedWorld(GameWorld::GetCurrent());
		GameWorld::SetCurrent(world);
	}
	else
	{
		ENGINE_EXCEPT(L"Event casting failed!");
	}
}