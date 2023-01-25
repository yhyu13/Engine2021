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
		float base_font_size = 18.0f;
		auto path = FileSystem::ResolveProtocol("$asset:ImGui/Roboto-Regular.ttf").string();
		m_Data.m_font_1k = io.Fonts->AddFontFromFileTTF(path.c_str(), base_font_size);
		m_Data.m_font_2k = io.Fonts->AddFontFromFileTTF(path.c_str(), 1.414f * base_font_size);
		m_Data.m_font_4k = io.Fonts->AddFontFromFileTTF(path.c_str(), 2.0f * base_font_size);
	}
	{
		auto queue = EventQueue<EditorEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<_3DEditorLayer>(this, EditorEventType::EDITOR_SWITCH_TO_EDITING_MODE, &_3DEditorLayer::_ON_EDITOR_SWITCH_TO_EDITING_MODE));
		ManageEventSubHandle(queue->Subscribe<_3DEditorLayer>(this, EditorEventType::EDITOR_SWITCH_TO_GAME_MODE, &_3DEditorLayer::_ON_EDITOR_SWITCH_TO_GAME_MODE));
	
		Engine::GetInstance()->EventQueueUpdate().Connect([](double dt) 
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
			LOCK_GUARD_GAMEWORLD_READYONLY(GameWorld::GetCurrent());
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
	ImFont* font;
	{
		if (auto height = Engine::GetWindow()->GetHeight(); height > 2000)
		{
			ImGuiUtil::ScaleAllSizesFromBase(2);
			font = m_Data.m_font_4k;
		}
		else if (height > 1400)
		{
			ImGuiUtil::ScaleAllSizesFromBase(1.414);
			font = m_Data.m_font_2k;
		}
		else
		{
			ImGuiUtil::ScaleAllSizesFromBase(1);
			font = m_Data.m_font_1k;
		}
	}
	ImGui::PushFont(font);
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
		GameWorld::GetCurrent()->PreRenderPass(ts);
		m_Data.mainRenderPipeline(ts);
		GameWorld::GetCurrent()->PostRenderPass(ts);
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

			GPU_NAMED_SCOPE(_3DEditorLayer);
			{
				Renderer3D::BeginRendering(cam);
				{
					GPU_NAMED_SCOPE(Shadow_Pass);
					GPU_TIME(Shadow_Pass);
					ENG_TIME("Shadow pass");
					Renderer3D::BeginShadowing(cam, f_render_opaque, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndShadowing();
				}
				{
					GPU_NAMED_SCOPE(Opaque_Scene_pass);
					GPU_TIME(Opaque_Scene_pass);
					ENG_TIME("Opaque Scene pass");
					Renderer3D::BeginOpaqueScene(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueScene();
				}
				{
					GPU_NAMED_SCOPE(Opaque_Lighting_pass);
					GPU_TIME(Opaque_Lighting_pass);
					ENG_TIME("Opaque Lighting pass");
					Renderer3D::BeginOpaqueLighting(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueLighting();
				}
				{
					GPU_NAMED_SCOPE(Transparent_Scene_pass);
					GPU_TIME(Transparent_Scene_pass);
					ENG_TIME("Transparent Scene pass");
					Renderer3D::BeginTransparentSceneAndLighting(cam, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndTransparentSceneAndLighting();
				}
				{
					GPU_NAMED_SCOPE(Postprocessing_pass);
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
		GameWorld::RemoveWorld(GameWorld::GetCurrent());
		GameWorld::SetCurrent(world);
	}
	else
	{
		ENGINE_EXCEPT(L"Event casting failed!");
	}
}
