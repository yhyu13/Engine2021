#pragma once

#include "engine/core/EngineCore.h"
#include "engine/input/InputManager.h"
#include "engine/renderer/GraphicsContext.h"

#include <json/json.h>


struct GLFWwindow;

namespace longmarch 
{
	typedef std::function<void(int IsFocussed)> InterruptHandler;

	struct ENGINE_API WindowProperties 
	{
		int m_xpos; // Upper left corner position in the whole screen
		int m_ypos; // Upper left corner position in the whole screen
		int m_width; // Width of the window, could be different from resolution
		int m_height; // Height of the window, could be different from resolution
		int m_resolutionX; // X Resolution, used in render target. Could be different from window width
		int m_resolutionY; // Y Resolution, used in render target. Could be different from window height
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
			m_xpos(0),
			m_ypos(0),
			m_width(width),
			m_height(height),
			m_title(title),
			m_resolutionX(width),
			m_resolutionY(height),
			m_monitorWidth(width),
			m_monitorHeight(height)
		{
			m_input = InputManager::GetInstance();
			m_input->SetMouseMaxPositions(static_cast<float>(width), static_cast<float>(height));
		}
	};

	class Engine;

	class ENGINE_API Window 
	{
	public:
		enum class CURSOR_MODE : int32_t
		{
			None = 0,
			NORMAL, // Disaply cursor and cursor has limited movement within the window
			HIDDEN, // Deos not disaply cursor and cursor has limited movement within the window
			HIDDEN_AND_FREE // Does not display cursor but cursor is still available with unlimited movement
		};

	public:
		explicit Window(const Json::Value& engineConfiguration);
		virtual ~Window();

		void Update(double dt);
		void Render();
		void Shutdown();
		bool ShouldExit();
		void UpdateTitle(const std::string& title);

		void ToggleFullScreen(int mode); /* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */

		void SetVisible(bool visible); //!< Set window hide or visible
		void SetResolution(int n);
		void SetVSync(bool on);
		void SetGPUSync(bool on);
		void SetCursorMode(CURSOR_MODE mode);
		CURSOR_MODE GetCursorMode();

		void ShowMessageBox(const std::wstring& title, const std::wstring& message);

		inline GLFWwindow* GetNativeWindow() const { return m_window; }
		inline WindowProperties& GetWindowProperties() { return m_windowProperties; }
		inline unsigned int GetWidth() const { return m_windowProperties.m_width; }
		inline unsigned int GetHeight() const { return m_windowProperties.m_height; }
		inline void SetInterruptHandler(const InterruptHandler& handler) { m_windowProperties.m_interruptHandler = handler; };

		static std::shared_ptr<Window> InitializeWindow(const Json::Value& engineConfiguration);

	private:
		friend Engine;
		
		void Init(const Json::Value& windowConfiguration);

	private:
		WindowProperties m_windowProperties;
		GLFWwindow* m_window;
		std::unique_ptr<GraphicsContext> m_context;
	};
}