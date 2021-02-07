#pragma once

#include "engine/core/EngineCore.h"
#include "engine/input/InputManager.h"

#include <json/json.h>

struct GLFWwindow;

namespace longmarch {
	typedef std::function<void(int IsFocussed)> InterruptHandler;

	struct ENGINE_API WindowProperties {
		int m_upperleft_xpos;
		int m_upperleft_ypos;
		int m_width;
		int m_height;
		int m_resolutionX;
		int m_resolutionY;
		std::string m_title;
		InputManager* m_input = nullptr;
		bool IsResizable = true;
		int IsFullScreen = 1; /* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */
		bool IsVSync = true;
		bool IsCPUGPUSync = true; 
		InterruptHandler m_interruptHandler = nullptr;

		std::pair<unsigned int, unsigned int> m_Res1;
		std::pair<unsigned int, unsigned int> m_Res2;
		std::pair<unsigned int, unsigned int> m_Res3;
		std::pair<unsigned int, unsigned int> m_Res4;

		int m_monitorWidth;
		int m_monitorHeight;

		WindowProperties(int width = 1280, int height = 720, std::string title = "ENGINE GSWY")
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
		inline WindowProperties& GetWindowProperties() { return m_windowProperties; }
		inline unsigned int GetWidth() const { return m_windowProperties.m_width; }
		inline unsigned int GetHeight() const { return m_windowProperties.m_height; }
		inline void SetInterruptHandler(const InterruptHandler& handler) { m_windowProperties.m_interruptHandler = handler; };

		static std::shared_ptr<Window> InitializeWindow(const Json::Value& engineConfiguration);
	private:
		void Init(const Json::Value& windowConfiguration);
	private:
		WindowProperties m_windowProperties;
		GLFWwindow* m_window;
	};
}