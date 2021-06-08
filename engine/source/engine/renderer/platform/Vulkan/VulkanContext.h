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
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();

	private:
		// Physical device
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool IsComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
		QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device);
		bool IsDeviceSuitable(const vk::PhysicalDevice& device);

	private:
		GLFWwindow* m_WindowHandle;
		vk::UniqueInstance m_instance;
		vk::SurfaceKHR m_surface;

		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;

		vk::Queue m_graphicsQueue;
		vk::Queue m_presentQueue;

		VkDebugUtilsMessengerEXT m_debugCallback;
	};
}


