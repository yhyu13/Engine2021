#pragma once
#include "engine/EngineEssential.h"
#include "engine/layer/LayerStack.h"
#include "engine/ui/ImGuiDriver.h"
#include "engine/window/Window.h"
#include "engine/framerate-controller/FramerateController.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "engine/events/EventQueue.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

namespace longmarch
{
	template<typename... Args>
	struct ConnectableUpdater
	{
		NONCOPYABLE(ConnectableUpdater);
		ConnectableUpdater() = default;
		size_t Connect(const std::function<void(Args...)>& callback)
		{
			auto index = m_updaters.size();
			m_updaters.emplace_back(callback);
			return index;
		}
		void Disconnect(size_t index)
		{
			m_updaters.erase(m_updaters.begin() + index);
		}
		void Update(Args... args)
		{
			for (auto& update : m_updaters)
			{
				update(args...);
			}
		}
	private:
		std::vector<std::function<void(Args...)>> m_updaters;
	};

	/**
	 * @brief The core engine class
	 *
	 * @author Dushyant Shukla (dushyant.shukla@digipen.edu | 60000519), Hang Yu (yohan680919@gmail.com)
	 */
	class ENGINE_API Engine : public BaseEventSubHandleClass
	{
	public:
		enum class ENGINE_MODE : uint8_t
		{
			EDITING = 0, // Editing a static scene
			INGAME,		 // Game is running
			INGAME_EDITING, // Editing a paused game
			INGAME_FREEROAM,// Free camera while game is running
			NUM
		};

	public:
		NONCOPYABLE(Engine);
		Engine();
		virtual ~Engine();
		virtual void Init();
		void Run();

		inline auto& PreUpdate() { return m_preUpdate; }
		inline auto& EventQueueUpdate() { return m_eventQueueUpdate; }
		inline auto& LateUpdate() { return m_lateUpdate; }
		inline auto& Render() { return m_renderUpdate; }
		inline auto& PostRenderUpdate() { return m_postRenderUpdate; }

		inline void SwitchCurrentLayer(Layer::LAYER_TYPE layer) { m_LayerStack.SwitchCurrentLayer(layer); }
		inline void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.PushLayer(layer); }
		inline void PushOverlay(const std::shared_ptr<Layer>& layer) { m_LayerStack.PushOverlay(layer); }

	private:
		void _ON_ENG_WINDOW_QUIT(EventQueue<EngineEventType>::EventPtr e);
		void _ON_CURSOR_SWITCH_MODE(EventQueue<EngineEventType>::EventPtr e);
		void _ON_TOGGLE_GPUSYNC(EventQueue<EngineGraphicsEventType>::EventPtr e);
		void _ON_TOGGLE_WINDOW_INTERRUTPTION_HANDLE(EventQueue<EngineSettingEventType>::EventPtr e);
		void _ON_TOGGLE_VSYNC(EventQueue<EngineGraphicsEventType>::EventPtr e);

	private:
		std::shared_ptr<Window> m_engineWindow{ nullptr };
		ConnectableUpdater<> m_preUpdate;
		ConnectableUpdater<double> m_eventQueueUpdate;
		ConnectableUpdater<double> m_lateUpdate;
		ConnectableUpdater<> m_renderUpdate;
		ConnectableUpdater<> m_postRenderUpdate;
		LayerStack m_LayerStack;
		ENGINE_MODE m_engineMode{ ENGINE_MODE::EDITING };
		unsigned int m_maxFrameRate{ 60u };
		bool m_shouldQUit{ false };
		bool m_isPaused{ false }; 
		bool m_isWindowFocused{ true };
		bool m_enable_pause_on_unfocused{ false };

	public:
		inline static GraphicsContext* GetGraphicsContext() { return s_instance->m_engineWindow->GetWindowProperties().m_context; }
		inline static std::shared_ptr<Window> GetWindow() { return s_instance->m_engineWindow; }
		inline static Engine* GetInstance() { return s_instance; }

		inline static void SetPaused(bool b) { s_instance->m_isPaused = (b); }
		inline static bool GetPaused() { return s_instance->m_isPaused; }

		inline static void SetQuit(bool b) { s_instance->m_shouldQUit = (b); }
		inline static bool GetQuit() { return s_instance->m_shouldQUit; }

		inline static void SetEngineMode(ENGINE_MODE mode) { s_instance->m_engineMode = mode; }
		inline static ENGINE_MODE GetEngineMode() { return s_instance->m_engineMode; }

		inline static void SetMaxFrameRate(unsigned int newMax) { FramerateController::GetInstance()->SetMaxFrameRate(newMax); }
		//! Return frame time in seconds
		inline static double GetFrameTime() { return FramerateController::GetInstance()->GetFrameTime(); }

		//! Return total time in seconds after the engine is created.
		static double GetTotalTime();
		static void ShowMessageBox(const std::wstring& title, const std::wstring& message);
		static void OnInterruption(int isFocussed);				

	private:
		inline static Engine* s_instance{ nullptr };
	};

	//! Must implement this function in your application.cpp
	Engine* CreateEngineApplication();
}
