#include "engine-precompiled-header.h"
#include "ImGuiDriver.h"
#include "engine/Engine.h"
#include "engine/renderer/platform/OpenGL/OpenGLContext.h"
#include "engine/renderer/platform/Vulkan/VulkanContext.h"

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
	bool s_init{ false };
	// TODO: move imgui driver to be a member variable of the window
	int s_api{ 0 };
	void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

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
		GLFWwindow* nativeWindow = static_cast<GLFWwindow*>(window->GetNativeWindow());
		s_api = window->GetWindowProperties().m_api;
		
		switch (s_api)
		{
		case 0:
		{
			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForOpenGL(nativeWindow, true);
			ImGui_ImplOpenGL3_Init("#version 450");
		}
			break;
		case 1:
		{
			auto context = GET_VK_CONTEXT();
			auto wd = context->GetVulkan_Window();
			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForVulkan(nativeWindow, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = context->s_Instance;
			init_info.PhysicalDevice = context->m_PhysicalDevice;
			init_info.Device = context->m_Device;
			init_info.QueueFamily = context->m_GraphicQueueIndices.graphicsFamily.value();
			init_info.Queue = context->m_GraphicsQueue;
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = context->m_DescriptorPool;
			init_info.Allocator = context->m_Allocator;
			init_info.MinImageCount = 2;
			init_info.ImageCount = wd->ImageCount;
			init_info.CheckVkResultFn = check_vk_result;
			ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
			ImGui::UploadFontTexture();
		}
			break;
		}
		s_init = true;
	}

	void ImGuiDriver::ShutDown()
	{
		if (s_init)
		{
			switch (s_api)
			{
			case 0:
				ImGui_ImplOpenGL3_Shutdown();
				break;
			case 1:
				ImGui_ImplVulkan_Shutdown();
				break;
			}
			ImGui_ImplGlfw_Shutdown();
			ImPlot::DestroyContext();
			ImGui::DestroyContext();
			s_init = false;
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
		io.DisplaySize = ImVec2(engine->GetWindow()->GetWidth(), engine->GetWindow()->GetHeight());
		const bool minimized = io.DisplaySize.x <= 0 && io.DisplaySize.y <= 0;
		//Rendering
		ImGui::Render();
		auto drawData = ImGui::GetDrawData();
		switch (s_api)
		{
		case 0:
			if (!minimized)
			{
				ImGui_ImplOpenGL3_RenderDrawData(drawData);
			}

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
			break;
		case 1:
			if (!minimized)
			{
				auto window = Engine::GetWindow();
				auto context = GET_VK_CONTEXT();
				auto wd = context->GetVulkan_Window();
				auto g_Device = context->m_Device;
				auto g_Queue = context->m_GraphicsQueue;
				{
					VkResult err;

					VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
					VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
					err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
					if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
					{
						context->RebuildSwapChain();
						return;
					}
					check_vk_result(err);

					auto fd = &wd->Frames[wd->FrameIndex];
					{
						err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
						check_vk_result(err);

						err = vkResetFences(g_Device, 1, &fd->Fence);
						check_vk_result(err);
					}
					{
						err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
						check_vk_result(err);
						VkCommandBufferBeginInfo info = {};
						info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
						info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
						err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
						check_vk_result(err);
					}
					{
						VkRenderPassBeginInfo info = {};
						info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
						info.renderPass = wd->RenderPass;
						info.framebuffer = fd->Framebuffer;
						info.renderArea.extent.width = wd->Extent.width;
						info.renderArea.extent.height = wd->Extent.height;
						info.clearValueCount = 1;
						info.pClearValues = &wd->ClearValue;
						vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
					}

					// Record dear imgui primitives into command buffer
					ImGui_ImplVulkan_RenderDrawData(drawData, fd->CommandBuffer);

					// Submit command buffer
					vkCmdEndRenderPass(fd->CommandBuffer);
					{
						VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						VkSubmitInfo info = {};
						info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
						info.waitSemaphoreCount = 1;
						info.pWaitSemaphores = &image_acquired_semaphore;
						info.pWaitDstStageMask = &wait_stage;
						info.commandBufferCount = 1;
						info.pCommandBuffers = &fd->CommandBuffer;
						info.signalSemaphoreCount = 1;
						info.pSignalSemaphores = &render_complete_semaphore;

						err = vkEndCommandBuffer(fd->CommandBuffer);
						check_vk_result(err);
						err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
						check_vk_result(err);
					}
				}
			}

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
			break;
		}
	}
}

#include "engine/renderer/platform/Vulkan/VulkanContext.h"

namespace ImGui
{
	void UploadFontTexture()
	{
		switch (longmarch::s_api)
		{
		case 0:
			// Nothing to do with OpenGL
			break;
		case 1:
		{
			// For vulkan, we need to create a command buffer that upload all current fonts,
			// every time we add new font to imgui
			auto window = Engine::GetWindow();
			auto context = GET_VK_CONTEXT();
			
			// Create temporal command buffer
			auto commandBuffer = context->BeginSingleTimeCommands();
			// Upload font texture by filling the command buffer
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			// Submit temporal command buffer
			context->EndSingleTimeCommands(commandBuffer);
			// Unload font texture
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
			break;
		}
	}
}