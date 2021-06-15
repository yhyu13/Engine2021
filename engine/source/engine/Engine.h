#pragma once
#include "engine/EngineEssential.h"
#include "layer/LayerStack.h"
#include "engine/ui/ImGuiDriver.h"
#include "engine/window/Window.h"
#include "engine/framerate-controller/FramerateController.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "engine/events/EventQueue.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

namespace longmarch
{
	struct ConnectableUpdater : public BaseAtomicClassNC
	{
		NONCOPYABLE(ConnectableUpdater);
		ConnectableUpdater() = default;
		size_t Connect(const std::function<void()>& callback)
		{
			LOCK_GUARD_NC();
			auto index = m_updaters.size();
			m_updaters.emplace_back(callback);
			return index;
		}
		void Disconnect(size_t index)
		{
			LOCK_GUARD_NC();
			m_updaters.erase(m_updaters.begin() + index);
		}
		void Update()
		{
			LOCK_GUARD_NC();
			for (auto& update : m_updaters)
			{
				update();
			}
		}
	private:
		std::vector<std::function<void()>> m_updaters;
	};

	struct ConnectableUpdater2 : public BaseAtomicClassNC
	{
		NONCOPYABLE(ConnectableUpdater2);
		ConnectableUpdater2() = default;
		size_t Connect(const std::function<void(double)>& callback)
		{
			LOCK_GUARD_NC();
			auto index = m_updaters.size();
			m_updaters.emplace_back(callback);
			return index;
		}
		void Disconnect(size_t index)
		{
			LOCK_GUARD_NC();
			m_updaters.erase(m_updaters.begin() + index);
		}
		void Update(double dt)
		{
			LOCK_GUARD_NC();
			for (auto& update : m_updaters)
			{
				update(dt);
			}
		}
	private:
		std::vector<std::function<void(double)>> m_updaters;
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

		inline ConnectableUpdater& PreUpdate() { return m_preUpdate; }
		inline ConnectableUpdater2& Update() { return m_update; }
		inline ConnectableUpdater2& LateUpdate() { return m_lateUpdate; }
		inline ConnectableUpdater& Render() { return m_renderUpdate; }
		inline ConnectableUpdater& PostRenderUpdate() { return m_postRenderUpdate; }

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
		ConnectableUpdater m_preUpdate;
		ConnectableUpdater2 m_update;
		ConnectableUpdater2 m_lateUpdate;
		ConnectableUpdater m_renderUpdate;
		ConnectableUpdater m_postRenderUpdate;
		LayerStack m_LayerStack;
		Timer m_timer;
		std::shared_ptr<Window> m_engineWindow{ nullptr };
		ENGINE_MODE m_engineMode{ ENGINE_MODE::EDITING };
		unsigned int m_maxFrameRate{ 60u };
		bool m_shouldQUit{ false };
		bool m_isPaused{ false }; 
		bool m_isWindowFocused{ true };
		bool m_enable_pause_on_unfocused{ false };

	public:
		inline static std::shared_ptr<Window> GetWindow() { return s_instance->m_engineWindow; }
		inline static Engine* GetInstance() { return s_instance; }
		inline static double GetTotalTime() { return s_instance->m_timer.Mark(); }

		inline static void SetPaused(bool b) { s_instance->m_isPaused = (b); }
		inline static bool GetPaused() { return s_instance->m_isPaused; }

		inline static void SetQuit(bool b) { s_instance->m_shouldQUit = (b); }
		inline static bool GetQuit() { return s_instance->m_shouldQUit; }

		inline static void SetEngineMode(ENGINE_MODE mode) { s_instance->m_engineMode = mode; }
		inline static ENGINE_MODE GetEngineMode() { return s_instance->m_engineMode; }

		static void SetMaxFrameRate(unsigned int newMax) { FramerateController::GetInstance()->SetMaxFrameRate(newMax); }
		//! Return frame time in seconds
		static double GetFrameTime() { return FramerateController::GetInstance()->GetFrameTime(); }

		static void ShowMessageBox(const std::wstring& title, const std::wstring& message);
		static void OnInterruption(int isFocussed);				

	private:
		inline static Engine* s_instance{ nullptr };
	};

	//! Must implement this function in your application.cpp
	Engine* CreateEngineApplication();
}
