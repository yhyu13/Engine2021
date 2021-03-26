#include "engine-precompiled-header.h"
#include "engine/window/Window.h"

#if defined(WIN32) || defined(WINDOWS_APP)
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace longmarch {
	std::shared_ptr<Window> Window::InitializeWindow(const Json::Value& windowConfiguration) {
		return std::make_shared<Window>(windowConfiguration);
	}

	void Window::Update(double dt) {
		glfwPollEvents();
	}

	void Window::Render()
	{
		glfwSwapBuffers(m_window);
		if (m_windowProperties.IsCPUGPUSync)
			glFinish();
	}

	void Window::UpdateTitle(std::string title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void Window::ToggleFullScreen(int mode)
	{
		glfwWindowHint(GLFW_RESIZABLE, m_windowProperties.IsResizable);
		/* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */
		if (mode != 2)
		{
			glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(m_window, (mode == 0) ? glfwGetPrimaryMonitor() : nullptr, 0, 0, m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY, GLFW_DONT_CARE);

			int w, h;
			glfwGetWindowSize(m_window, &w, &h);
			m_windowProperties.m_width = w;
			m_windowProperties.m_height = h;

			printf("Full Screen : width = %d, height = %d\n", w, h);

			int l, r, t, b;
			glfwGetWindowFrameSize(m_window, &l, &t, &r, &b);
			printf("Frame Size : left = %d, top = %d right = %d, bottom = %d\n", l, t, r, b);
		}
		else
		{
			glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			int x = 0, y = 0, w = 1, h = 1;
			glfwSetWindowMonitor(m_window, nullptr, x, y, w, h, GLFW_DONT_CARE);

			int l, r, t, b;
			glfwGetWindowFrameSize(m_window, &l, &t, &r, &b);
			printf("Frame Size : left = %d, top = %d right = %d, bottom = %d\n", l, t, r, b);

			//int x, y, w, h;
			glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, &w, &h);
			glfwSetWindowMonitor(m_window, nullptr, x, y, m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY, GLFW_DONT_CARE);

			m_windowProperties.m_width = m_windowProperties.m_resolutionX;
			m_windowProperties.m_height = m_windowProperties.m_resolutionY;
			printf("No Full Screen : posX = %d, posY = %d width = %d, height = %d\n", x, y, w, h);
		}
		SetVSync(m_windowProperties.IsVSync);
		m_windowProperties.IsFullScreen = mode;
		int width = m_windowProperties.m_width;
		int height = m_windowProperties.m_height;
		DEBUG_PRINT(Str("Window Size : %d x %d\n", width, height));

		glfwGetFramebufferSize(m_window, &width, &height);
		DEBUG_PRINT(Str("Framebuffer Size : %d x %d\n", width, height));

		m_windowProperties.m_input->SetMouseMaxPositions(m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY);
	}

	void Window::SetVisible(bool visible)
	{
		if (!visible)
		{
			glfwHideWindow(m_window);
		}
		else
		{
			glfwShowWindow(m_window);
		}
	}

	void Window::SetResolution(int n)
	{
		switch (n)
		{
		case 0:
		{
			m_windowProperties.m_resolutionX = m_windowProperties.m_Res1.first;
			m_windowProperties.m_resolutionY = m_windowProperties.m_Res1.second;
		}
		break;

		case 1:
		{
			m_windowProperties.m_resolutionX = m_windowProperties.m_Res2.first;
			m_windowProperties.m_resolutionY = m_windowProperties.m_Res2.second;
		}
		break;

		case 2:
		{
			m_windowProperties.m_resolutionX = m_windowProperties.m_Res3.first;
			m_windowProperties.m_resolutionY = m_windowProperties.m_Res3.second;
		}
		break;

		case 3:
		{
			m_windowProperties.m_resolutionX = m_windowProperties.m_Res4.first;
			m_windowProperties.m_resolutionY = m_windowProperties.m_Res4.second;
		}
		break;

		default:
		{
			m_windowProperties.m_resolutionX = m_windowProperties.m_width;
			m_windowProperties.m_resolutionY = m_windowProperties.m_height;
		}
		break;
		}

		glfwWindowHint(GLFW_RESIZABLE, m_windowProperties.IsResizable);

		/* 0-Full screen, 1- Borderless full screen 2- Windowed mode  */
		if (m_windowProperties.IsFullScreen != 2)
		{
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(m_window, (m_windowProperties.IsFullScreen == 0) ? glfwGetPrimaryMonitor() : nullptr, 0, 0, m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY, GLFW_DONT_CARE);
		}
		else
		{
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			int x, y;
			glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, nullptr, nullptr);
			glfwSetWindowMonitor(m_window, nullptr, x, y, m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY, GLFW_DONT_CARE);
		}
		SetVSync(m_windowProperties.IsVSync);
		m_windowProperties.m_input->SetMouseMaxPositions(m_windowProperties.m_resolutionX, m_windowProperties.m_resolutionY);
	}

	void Window::SetVSync(bool on)
	{
		if (on)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		m_windowProperties.IsVSync = on;
	}

	void Window::SetGPUSync(bool on)
	{
		m_windowProperties.IsCPUGPUSync = on;
	}

	void Window::ShowMessageBox(const std::wstring& title, const std::wstring& message)
	{
#if defined(WIN32) || defined(WINDOWS_APP)
		if (auto window = glfwGetWin32Window(m_window); window)
		{
			MessageBox(window, message.c_str(), title.c_str(), MB_OK);
		}
#endif
	}

	void Window::Shutdown() {
		glfwDestroyWindow(m_window);
	}

	bool Window::ShouldExit() {
		return glfwWindowShouldClose(m_window);
	}

	Window::Window(const Json::Value& windowConfiguration) {
		Init(windowConfiguration);
	}

	Window::~Window() {
		Shutdown();
	}

	void Window::Init(const Json::Value& windowConfiguration) {
#if defined(WIN32) || defined(WINDOWS_APP)
		// This api call make the process be aware of dpi
		// i.e. managing dpi by ourself instead of Desktop Window Manager(DWM)
		// i.e. ignoring user dpi scales which are common to be 125%, 150%, etc
		// Not making this api call would result in the window we created to be scaled with
		// user dpi setting (e.g. "Change Display Setting" in Win10).
		//https://docs.microsoft.com/en-us/windows/win32/api/windef/ne-windef-dpi_awareness
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
#endif
		bool hide = windowConfiguration["Hide-On-Creation"].asBool();
		m_windowProperties.IsResizable = windowConfiguration["Resizable"].asBool();
		m_windowProperties.m_width = windowConfiguration["Width"].asInt();
		m_windowProperties.m_height = windowConfiguration["Height"].asInt();
		m_windowProperties.m_title = windowConfiguration["Title"].asString();
		m_windowProperties.IsFullScreen = windowConfiguration["Full-screen"].asInt();
		m_windowProperties.IsVSync = windowConfiguration["V-sync"].asBool();
		m_windowProperties.IsCPUGPUSync = windowConfiguration["GPU-sync"].asBool();

		m_windowProperties.m_input = InputManager::GetInstance();
		m_windowProperties.m_input->SetMouseMaxPositions(m_windowProperties.m_width, m_windowProperties.m_height);

		m_windowProperties.m_Res1 = { windowConfiguration["Resolution1_X"].asInt(), windowConfiguration["Resolution1_Y"].asInt() };
		m_windowProperties.m_Res2 = { windowConfiguration["Resolution2_X"].asInt(), windowConfiguration["Resolution2_Y"].asInt() };
		m_windowProperties.m_Res3 = { windowConfiguration["Resolution3_X"].asInt(), windowConfiguration["Resolution3_Y"].asInt() };
		m_windowProperties.m_Res4 = { windowConfiguration["Resolution4_X"].asInt(), windowConfiguration["Resolution4_Y"].asInt() };

		int success = glfwInit();
		ASSERT(success >= 0, "Failed to initialize GLFW!");

		auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		m_windowProperties.m_monitorWidth = mode->width;
		m_windowProperties.m_monitorHeight = mode->height;

		m_windowProperties.m_resolutionX = m_windowProperties.m_width;
		m_windowProperties.m_resolutionY = m_windowProperties.m_height;

		glfwWindowHint(GLFW_RESIZABLE, m_windowProperties.IsResizable);
		glfwWindowHint(GLFW_VISIBLE, !hide);

		if (m_windowProperties.IsFullScreen != 2)
		{
			//Full Screen Settings
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

			// Update window width and height
			m_windowProperties.m_resolutionX = m_windowProperties.m_width = mode->width;
			m_windowProperties.m_resolutionY = m_windowProperties.m_height = mode->height;

			m_window = glfwCreateWindow(mode->width, mode->height, m_windowProperties.m_title.c_str(), (m_windowProperties.IsFullScreen == 0) ? monitor : nullptr, nullptr);
			m_windowProperties.m_input->SetMouseMaxPositions(mode->width, mode->height);
		}
		else
		{
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			m_window = glfwCreateWindow(m_windowProperties.m_width, m_windowProperties.m_height, m_windowProperties.m_title.c_str(), nullptr, nullptr);
			m_windowProperties.m_input->SetMouseMaxPositions(m_windowProperties.m_width, m_windowProperties.m_height);
		}

		ASSERT(m_window != nullptr, "Failed to create window!");

		// Set V-Sync
		SetVSync(m_windowProperties.IsVSync);
		// Set min window size
		glfwSetWindowSizeLimits(m_window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
		// Make window context
		glfwMakeContextCurrent(m_window);
		// Register user pointer forsake of callbacks
		glfwSetWindowUserPointer(m_window, &m_windowProperties);
		// Get window pos
		glfwGetWindowPos(m_window, &m_windowProperties.m_xpos, &m_windowProperties.m_ypos);

		success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ASSERT(success >= 0, "Could not initialize Glad!");

		// Put info to Log
		ENGINE_INFO(" OpenGL Info:");
		ENGINE_INFO(" Vendor: {0}", glGetString(GL_VENDOR));
		ENGINE_INFO(" Renderer: {0}", glGetString(GL_RENDERER));
		ENGINE_INFO(" Version: {0}", glGetString(GL_VERSION));

		// Callbacks
		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scanCode, int action, int mods) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
			{
				properties.m_input->UpdateKeyboardState(key, true, true);
				break;
			}
			case GLFW_RELEASE:
			{
				properties.m_input->UpdateKeyboardState(key, false, false);
				break;
			}
			default:
				break;
			}
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
			{
				properties.m_input->UpdateMouseButtonState(button, true);
				break;
			}
			case GLFW_RELEASE:
			{
				properties.m_input->UpdateMouseButtonState(button, false);
				break;
			}
			default:
				break;
			}
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double positionX, double positionY) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			properties.m_input->UpdateCursorPosition(positionX, positionY);
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			properties.m_input->UpdateMouseScroll(xoffset, yoffset);
		});

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			properties.m_width = width;
			properties.m_height = height;
			properties.m_input->SetMouseMaxPositions(width, height);
		});

		glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int upperleft_xpos, int upperleft_ypos)
		{
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			properties.m_xpos = upperleft_xpos;
			properties.m_ypos = upperleft_ypos;
		}
		);

		glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focussed)
		{
			WindowProperties& properties = *(WindowProperties*)glfwGetWindowUserPointer(window);
			if (properties.m_interruptHandler)
			{
				properties.m_interruptHandler(focussed);
			}
		});

		glfwSetJoystickCallback([](int gamepadid, int state) {
			ENGINE_INFO("game pad id: {0} state: {1}", gamepadid, state == GLFW_CONNECTED ? "connected" : "disconnected");
		});
	}
}