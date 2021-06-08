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
		void SetUpValidationLayer();
		void PickPhysicalDevice();
		void CreateLogicalDevice();

	private:
		GLFWwindow* m_WindowHandle;
		vk::UniqueInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugCallback;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vk::Queue m_graphicsQueue;
	};
}


