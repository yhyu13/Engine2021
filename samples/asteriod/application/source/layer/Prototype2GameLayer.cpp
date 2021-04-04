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

//***************************************************************************************
//! \file	   Prototype2GameLayer.cpp
//! \author    Hang Yu (hang.yu@digipen.edu | 60001119)
//! \date      05/13/2020
//! \copyright Copyright (C) 2020 DigiPen Institute of Technology.
//***************************************************************************************

#include "application-precompiled-header.h"
#include "Prototype2GameLayer.h"
#include "engine/audio/AudioManager.h"
#include "engine/ecs/header/header.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

#include "events/EventType.h"
#include "ecs/header/header.h"
#include "ui/header.h"

longmarch::Prototype2GameLayer::Prototype2GameLayer()
	: Layer("Prototype2GameLayer")
{
	Renderer3D::Init();
}

void longmarch::Prototype2GameLayer::Init()
{
	APP_TIME("App Initialization");
	InitFramework();
	InitGameWorld();
	BuildRenderPipeline();
	{
		// Register _3DEngineWidgetManager
		ServiceLocator::ProvideSingleton<_3DGameWidgetManager>(APP_WIG_MAN_NAME, MemoryManager::Make_shared<_3DGameWidgetManager>());
	}
	{
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		m_font = io.Fonts->AddFontFromFileTTF(FileSystem::ResolveProtocol("$asset:ImGui/ComicNeue-Bold.ttf").string().c_str(), 20.0f);
	}
	{
		auto queue = EventQueue<EngineIOEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::LOAD_SCENE_BEGIN, &Prototype2GameLayer::_ON_LOAD_SCENE_BEGIN));
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::LOAD_SCENE, &Prototype2GameLayer::_ON_LOAD_SCENE));
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::LOAD_SCENE_END, &Prototype2GameLayer::_ON_LOAD_SCENE_END));

		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::SAVE_SCENE_BEGIN, &Prototype2GameLayer::_ON_SAVE_SCENE_BEGIN));
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::SAVE_SCENE, &Prototype2GameLayer::_ON_SAVE_SCENE));
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineIOEventType::SAVE_SCENE_END, &Prototype2GameLayer::_ON_SAVE_SCENE_END));
	}
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<Prototype2GameLayer>(this, EngineEventType::ENG_WINDOW_INTERRUTPTION, &Prototype2GameLayer::_ON_WINDOW_INTERRUPT));
	}
}

/*
Load engine frameworks
*/

void longmarch::Prototype2GameLayer::InitFramework()
{
	ServiceLocator::Register<MainObjectFactory>("ObjectFactory");
	{
		ENG_TIME("Loading resources");
		LoadResources();
	}
}

/*
Load all game resources
*/

void longmarch::Prototype2GameLayer::LoadResources()
{
	auto objectFactory = ServiceLocator::GetSingleton<ObjectFactory>("ObjectFactory");
	objectFactory->LoadResources(("$asset:archetype/resource.json"), nullptr);
}

/*
Load start up screen
*/

void longmarch::Prototype2GameLayer::InitGameWorld()
{
	// 2, Init game world.
	// This step must happen after EventQueue clear as systems in mainGameWorld->Init() would register for event handler at that stage)

	auto filepath = ("$asset:archetype/scene-game.json");
	GameWorld::GetInstance(true, "", filepath);
}

void longmarch::Prototype2GameLayer::PreUpdate(double ts)
{
	{
		auto queue = EventQueue<GameEventType>::GetInstance();
		queue->Update(ts);
	}
	{
		auto queue = EventQueue<GameDebugEventType>::GetInstance();
		queue->Update(ts);
	}
	{
		auto queue = EventQueue<GameSettingEventType>::GetInstance();
		queue->Update(ts);
	}
}

void longmarch::Prototype2GameLayer::Update(double ts)
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadUpdate(ts);
#else
	GameWorld::GetCurrent()->Update(ts);
#endif // MULTITHREAD_UPDATE
}

void longmarch::Prototype2GameLayer::JoinAll()
{
#ifdef MULTITHREAD_UPDATE
	GameWorld::GetCurrent()->MultiThreadJoin();
#endif // MULTITHREAD_UPDATE
}

void longmarch::Prototype2GameLayer::PreRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PreRenderUpdate(ts);
}

void longmarch::Prototype2GameLayer::Render(double ts)
{
	switch (Engine::GetEngineMode())
	{
	case Engine::ENGINE_MODE::INGAME:
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

void longmarch::Prototype2GameLayer::PostRenderUpdate(double ts)
{
	GameWorld::GetCurrent()->PostRenderUpdate(ts);
}

void longmarch::Prototype2GameLayer::OnUpdate(double ts)
{
	double dt = GameWorld::GetCurrent()->IsPaused() ? 0.0 : ts;
	{
		{
			APP_TIME("Pre System update");
			PreUpdate(dt);
		}
		{
			APP_TIME("PreRender update");
			PreRenderUpdate(dt);
		}
		{
			APP_TIME("System update");
			Update(dt);
		}
		{
			APP_TIME("Render");
			Render(dt);
		}
		{
			JoinAll();
		}
		{
			APP_TIME("PostRender update");
			PostRenderUpdate(dt);
		}
	}
}

void longmarch::Prototype2GameLayer::BuildRenderPipeline()
{
	Renderer3D::BuildAllMesh();
	Renderer3D::BuildAllMaterial();
	Renderer3D::BuildAllTexture();

	m_Data.mainRenderPipeline = [this](double ts)
	{
		EntityType e_type;
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::INGAME:
			e_type = (EntityType)EngineEntityType::PLAYER_CAMERA;
			break;
		default:
			throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Engine mode is not valid!");
		}

		// TODO move render pipeline for INGame to application's layer, and use application layer in application mode
		auto camera = GameWorld::GetCurrent()->GetTheOnlyEntityWithType(e_type);
		auto cam = GameWorld::GetCurrent()->GetComponent<PerspectiveCameraCom>(camera)->GetCamera();
		switch (Engine::GetEngineMode())
		{
		case Engine::ENGINE_MODE::INGAME:
		{
			const auto& prop = Engine::GetWindow()->GetWindowProperties();
			cam->SetViewPort(Vec2u(0), Vec2u(prop.m_width, prop.m_height));
			if (prop.IsResizable)
			{
				cam->cameraSettings.aspectRatioWbyH = float(prop.m_width) / float(prop.m_height);
			}
		}
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
					APP_TIME("Shadow pass");
					Renderer3D::BeginShadowing(cam, f_render_opaque, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndShadowing();
				}
				{
					GPU_TIME(Opaque_Scene_pass);
					APP_TIME("Opaque Scene pass");
					Renderer3D::BeginOpaqueScene(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueScene();
				}
				{
					GPU_TIME(Opaque_Lighting_pass);
					APP_TIME("Opaque Lighting pass");
					Renderer3D::BeginOpaqueLighting(cam, f_render_opaque, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndOpaqueLighting();
				}
				{
					GPU_TIME(Transparent_Scene_pass);
					APP_TIME("Transparent Scene pass");
					Renderer3D::BeginTransparentSceneAndLighting(cam, f_render_translucent, f_setVFCullingParam, f_setDistanceCullingParam, f_setRenderShaderName);
					Renderer3D::EndTransparentSceneAndLighting();
				}
				{
					GPU_TIME(Postprocessing_pass);
					APP_TIME("Postprocessing pass");
					Renderer3D::BeginPostProcessing();
					Renderer3D::EndPostProcessing();
				}
				Renderer3D::EndRendering();
			}
		}
	};
}


void longmarch::Prototype2GameLayer::OnAttach()
{
}

void longmarch::Prototype2GameLayer::OnDetach()
{
}

void longmarch::Prototype2GameLayer::OnImGuiRender()
{
	APP_TIME("ImGUI Render");
	ImGui::PushFont(m_font);
	auto manager = ServiceLocator::GetSingleton<BaseGameWidgetManager>(APP_WIG_MAN_NAME);
	manager->BeginFrame();
	manager->RenderUI();
	manager->EndFrame();
	GameWorld::GetCurrent()->RenderUI();
	ImGui::PopFont();
}

void longmarch::Prototype2GameLayer::_ON_LOAD_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::Prototype2GameLayer::_ON_LOAD_SCENE(EventQueue<EngineIOEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineLoadSceneEvent>(e);
	auto& filepath = event->m_filepath;
	// Generate new game world instance, which frees the old world // TODO delayed remove old world
	GameWorld::GetInstance(event->m_makeCurrent, "", filepath);
}

void longmarch::Prototype2GameLayer::_ON_LOAD_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e)
{
	Renderer3D::BuildAllMesh();
	Renderer3D::BuildAllMaterial();
	Renderer3D::BuildAllTexture();
}

void longmarch::Prototype2GameLayer::_ON_SAVE_SCENE_BEGIN(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::Prototype2GameLayer::_ON_SAVE_SCENE(EventQueue<EngineIOEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineSaveSceneEvent>(e);
	auto& filepath = event->m_filepath;
	auto world = static_cast<GameWorld*>(event->m_gameworld);
	auto objectFactory = ServiceLocator::GetSingleton<ObjectFactory>("ObjectFactory");
	objectFactory->SaveGameWorldScene(filepath, world);
}

void longmarch::Prototype2GameLayer::_ON_SAVE_SCENE_END(EventQueue<EngineIOEventType>::EventPtr e)
{
}

void longmarch::Prototype2GameLayer::_ON_WINDOW_INTERRUPT(EventQueue<EngineEventType>::EventPtr e)
{
	auto event = std::static_pointer_cast<EngineWindowInterruptionEvent>(e);
	GameWorld::GetCurrent()->SetPause(!event->m_isFocused);
}