#include "engine-precompiled-header.h"
#include "Engine.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"
#include "engine/renderer/material/Material.h"
#include "engine/core/utility/Random.h"

namespace longmarch
{
	Engine::Engine()
	{
		s_instance = this;
	}

	Engine::~Engine()
	{
		ImGuiDriver::ShutDown();
		Logger::ShutDown();
		s_instance = nullptr;
	}

	void Engine::Init()
	{
		// Logger
		{
			Logger::Init();
			ENGINE_INFO("Initialized Engine Log!");
			APP_INFO("Initialized Application Log!");
		}
		auto config_path = FileSystem::Absolute("engine-config.json");
		const auto& engineConfiguration = FileSystem::GetCachedJsonCPP(config_path);
		// File system
		{
			const auto& path = engineConfiguration["path"];
			FileSystem::RegisterProtocol("$root:", FileSystem::Absolute(path["Root"].asString()));
			FileSystem::RegisterProtocol("$asset:", FileSystem::Absolute(path["Asset"].asString()));
			FileSystem::RegisterProtocol("$shader:", FileSystem::Absolute(path["Shader"].asString()));
			FileSystem::RegisterProtocol("$scene:", FileSystem::Absolute(path["Scene"].asString()));
			FileSystem::RegisterProtocol("$texture:", FileSystem::Absolute(path["Texture"].asString()));
			FileSystem::RegisterProtocol("$sound:", FileSystem::Absolute(path["Sound"].asString()));
		}
		// Window system
		{
			auto wd = Window::InitializeWindow(engineConfiguration["window"]);
			m_engineWindow = wd;
			m_engineWindow->SetInterruptHandler(Engine::OnInterruption);
			m_maxFrameRate = engineConfiguration["graphics"]["Max-framerate"].asInt();
			FramerateController::GetInstance()->SetMaxFrameRate(m_maxFrameRate);
		}
		// Engine config
		{
			m_enable_pause_on_unfocused = engineConfiguration["engine"]["Pause-on-unfocused"].asBool();
			switch (engineConfiguration["engine"]["Startup-mode"].asInt())
			{
			case 0:
				m_engineMode = ENGINE_MODE::EDITING;
				break;
			case 1:
				m_engineMode = ENGINE_MODE::INGAME;
				break;
			default:
				ENGINE_EXCEPT(L"Unknown startup mode!");
				break;
			}
			if (engineConfiguration["test"]["Reset-random-seed"].asBool())
			{
				RAND_SEED_SET_TIME;
			}
		}
		// Engine updaters
		{
			PreUpdate().Connect(AssetLoader::Update);
			Update().Connect([](double dt)
			{
				{
					auto queue = EventQueue<EngineEventType>::GetInstance();
					queue->Update(dt);
				}
				{
					auto queue = EventQueue<EngineGraphicsEventType>::GetInstance();
					queue->Update(dt);
				}
				{
					auto queue = EventQueue<EngineGraphicsDebugEventType>::GetInstance();
					queue->Update(dt);
				}
			});
			Update().Connect([this](double dt) 
			{
				m_engineWindow->Update(dt);
			});
			Render().Connect([this]() 
			{ 
				m_engineWindow->Render(); 
			});
			PostRenderUpdate().Connect(EngineException::Update);
			PostRenderUpdate().Connect([this]() 
			{
				// Check should exit
				if (!m_shouldQUit && m_engineWindow->ShouldExit())
				{
					m_shouldQUit = true;
				}
			});
		}

		{
			ImGuiDriver::Init(); //!< Initializing imgui layer after window creation
			Material::Init(); //!< Initializing material instances after window creation
			Random::Init(); //!< Initializing random engine
		} 
		{
			auto queue = EventQueue<EngineEventType>::GetInstance();
			{
				ManageEventSubHandle(queue->Subscribe<Engine>(s_instance, EngineEventType::ENG_WINDOW_QUIT, &Engine::_ON_ENG_WINDOW_QUIT));
				ManageEventSubHandle(queue->Subscribe<Engine>(s_instance, EngineEventType::ENG_WINDOW_CURSOR_SWITCH_MODE, &Engine::_ON_CURSOR_SWITCH_MODE));
			}
		}
		{
			auto queue = EventQueue<EngineGraphicsEventType>::GetInstance();
			{
				ManageEventSubHandle(queue->Subscribe<Engine>(s_instance, EngineGraphicsEventType::TOGGLE_VSYNC, &Engine::_ON_TOGGLE_VSYNC));
				ManageEventSubHandle(queue->Subscribe<Engine>(s_instance, EngineGraphicsEventType::TOGGLE_GPUSYNC, &Engine::_ON_TOGGLE_GPUSYNC));
			}
		}
		{
			auto queue = EventQueue<EngineSettingEventType>::GetInstance();
			{
				ManageEventSubHandle(queue->Subscribe<Engine>(s_instance, EngineSettingEventType::TOGGLE_WINDOW_INTERRUTPTION_HANDLE, &Engine::_ON_TOGGLE_WINDOW_INTERRUTPTION_HANDLE));
			}
		}
	}

	void Engine::_ON_ENG_WINDOW_QUIT(EventQueue<EngineEventType>::EventPtr e)
	{
		// Nothing
	}

	void Engine::_ON_CURSOR_SWITCH_MODE(EventQueue<EngineEventType>::EventPtr e)
	{
		auto event = std::static_pointer_cast<EngineCursorSwitchModeEvent>(e);
		s_instance->GetWindow()->SetCursorMode(Window::CURSOR_MODE(event->m_mode));
	}

	void Engine::_ON_TOGGLE_WINDOW_INTERRUTPTION_HANDLE(EventQueue<EngineSettingEventType>::EventPtr e)
	{
		auto event = std::static_pointer_cast<ToggleWindowInterrutpionEvent>(e);
		s_instance->m_enable_pause_on_unfocused = event->m_enable;
	}

	void longmarch::Engine::_ON_TOGGLE_VSYNC(EventQueue<EngineGraphicsEventType>::EventPtr e)
	{
		auto event = std::static_pointer_cast<ToggleVSyncEvent>(e);
		s_instance->GetWindow()->SetVSync(event->m_enable);
	}

	void longmarch::Engine::_ON_TOGGLE_GPUSYNC(EventQueue<EngineGraphicsEventType>::EventPtr e)
	{
		auto event = std::static_pointer_cast<ToggleGPUSyncEvent>(e);
		s_instance->GetWindow()->SetGPUSync(event->m_enable);
	}

	void Engine::Run() 
	{
		auto rateController = FramerateController::GetInstance();
		while (!Engine::GetQuit())
		{
			rateController->FrameStart();
			{
				Instrumentor::GetEngineInstance()->AddInstrumentorResult({ "Up Time", (m_timer.Mark()), "s" });

				// Pre update
				PreUpdate().Update();

				double dt = rateController->GetFrameTime();
				// Engine update
				Update().Update(dt);

				if (!Engine::GetPaused())
				{
					// Updating for the current layer
					for (auto& layer : *m_LayerStack.GetCurrentLayer())
					{
						layer->OnUpdate(dt);
					}

					// Render the imgui UI of the cureent layer
					ImGuiDriver::BeginFrame();
					for (auto& layer : *m_LayerStack.GetCurrentLayer())
					{
						layer->OnImGuiRender();
					}
					ImGuiDriver::EndFrame();

					// Engine post game layer update
					LateUpdate().Update(dt);

					// Window swap buffer
					Render().Update();
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<size_t>(rateController->GetTargetFrameTime() * 1e3)));
				}

				// Post update
				PostRenderUpdate().Update();
			}
			rateController->FrameEnd();
		}

		// Publish engine window quit event
		auto queue = EventQueue<EngineEventType>::GetInstance();
		auto e = MemoryManager::Make_shared<EngineWindowQuitEvent>();
		queue->Publish(e);
	}

	void longmarch::Engine::ShowMessageBox(const std::wstring& title, const std::wstring& message)
	{
		if (s_instance != nullptr)
		{
			if (auto wd = s_instance->GetWindow(); wd != nullptr)
			{
				wd->ShowMessageBox(title, message);
			}
		}
	}

	void longmarch::Engine::OnInterruption(int isFocussed)
	{
		s_instance->m_isWindowFocused = static_cast<bool>(isFocussed);

		if (s_instance->m_enable_pause_on_unfocused)
		{
			Engine::SetPaused(!s_instance->m_isWindowFocused);
		}
		auto queue = EventQueue<EngineEventType>::GetInstance();
		auto e = MemoryManager::Make_shared<EngineWindowInterruptionEvent>(s_instance->m_isWindowFocused);
		queue->Publish(e);
	}
}