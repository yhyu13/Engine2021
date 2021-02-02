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
}

void longmarch::Prototype2GameLayer::Init()
{
	APP_TIME("App Initialization");
	InitFramework();
	InitGameWorld();
	BuildTestScene();
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

void longmarch::Prototype2GameLayer::BuildTestScene()
{
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

void longmarch::Prototype2GameLayer::OnUpdate(double ts)
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