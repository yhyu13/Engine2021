#include "engine-precompiled-header.h"
#include "ImGuiDriver.h"
#include "../Engine.h"
#include "../window/Window.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/addons/implot/implot.h>

namespace longmarch 
{
	static int s_api{ 0 };

	void ImGuiDriver::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
		}

		auto window = Engine::GetWindow();
		GLFWwindow* wd = static_cast<GLFWwindow*>(window->GetNativeWindow());
		s_api = window->GetWindowProperties().m_api;
		
		switch (s_api)
		{
		case 0:
			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForOpenGL(wd, true);
			ImGui_ImplOpenGL3_Init("#version 450");
			break;
		case 1:
			throw NotImplementedException();
			break;
		}
	}

	void ImGuiDriver::ShutDown()
	{
		switch (s_api)
		{
		case 0:
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImPlot::DestroyContext();
			ImGui::DestroyContext();
			break;
		case 1:
			throw NotImplementedException();
			break;
		}
	}

	void ImGuiDriver::BeginFrame()
	{
		switch (s_api)
		{
		case 0:
			ImGui_ImplOpenGL3_NewFrame();
			break;
		case 1:
			ImGui_ImplVulkan_NewFrame();
			break;
		}
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiDriver::EndFrame()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto engine = Engine::GetInstance();
		io.DisplaySize = ImVec2((unsigned int)engine->GetWindow()->GetWidth(), (unsigned int)engine->GetWindow()->GetHeight());

		//Rendering
		ImGui::Render();

		switch (s_api)
		{
		case 0:
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
			break;
		case 1:
			throw NotImplementedException();
			break;
		}
	}
}