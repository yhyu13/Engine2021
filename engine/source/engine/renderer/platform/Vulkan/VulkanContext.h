#pragma once

#include "engine/renderer/GraphicsContext.h"
#include "VulkanUtil.h"

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace longmarch 
{
	class VulkanContext final : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		void CreateInstance();
		void SetUpValidationLayerDebugMessenger();

	private:
		GLFWwindow* m_WindowHandle;
		vk::UniqueInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugCallback;
	};
}


