#pragma once

#include "engine/core/EngineCore.h"
#include "engine/input/InputManager.h"

#include <json/json.h>

struct GLFWwindow;

namespace AAAAgames {
	typedef std::function<void(const int& isFocussed)> InterruptHandler;

	struct ENGINE_API WindowProperties {
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_resolutionX;
		unsigned int m_resolutionY;
		std::string m_title;
		InputManager* m_input = nullptr;
		int IsFullScreen = 1; /* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */
		InterruptHandler m_interruptHandler = nullptr;
		bool IsVSync = true;
		bool IsCPUGPUSync = true;

		std::pair<unsigned int, unsigned int> m_Res1;
		std::pair<unsigned int, unsigned int> m_Res2;
		std::pair<unsigned int, unsigned int> m_Res3;
		std::pair<unsigned int, unsigned int> m_Res4;

		unsigned int m_monitorWidth;
		unsigned int m_monitorHeight;

		WindowProperties(unsigned int width = 1280, unsigned int height = 720, std::string title = "ENGINE GSWY")
			:
			m_width(width),
			m_height(height),
			m_title(title),
			m_resolutionX(width),
			m_resolutionY(height),
			m_monitorWidth(width),
			m_monitorHeight(height)
		{
			m_input = InputManager::GetInstance();
			m_input->SetMouseMaxPositions(width, height);
		}
	};

	class Engine;
	class ENGINE_API Window {
	public:
		friend Engine;

		explicit Window(const Json::Value& engineConfiguration);
		~Window();

		void Update(double dt);
		void Render();
		void Shutdown();
		bool ShouldExit();
		void UpdateTitle(std::string title);

		void ToggleFullScreen(int mode); /* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */

		void SetVisible(bool visible); //!< Set window hide or visible
		void SetResolution(int n);
		void SetVSync(bool on);
		void SetGPUSync(bool on);

		void ShowMessageBox(const std::wstring& title, const std::wstring& message);

		inline GLFWwindow* GetNativeWindow() const { return m_window; }
		inline WindowProperties GetWindowProperties() const { return m_windowProperties; }
		inline unsigned int GetWidth() const { return m_windowProperties.m_width; }
		inline unsigned int GetHeight() const { return m_windowProperties.m_height; }
		inline void SetInterruptHandler(const InterruptHandler& handler) { m_windowProperties.m_interruptHandler = handler; };

		static std::shared_ptr<Window> InitializeWindow(const Json::Value& engineConfiguration);
	private:
		void Init(const Json::Value& windowConfiguration);
	private:
		WindowProperties m_windowProperties;
		GLFWwindow* m_window;
	public:
		inline static unsigned int width = { 1920 };
		inline static unsigned int height = { 1080 };
		inline static bool Resizable = { true };
	};
}