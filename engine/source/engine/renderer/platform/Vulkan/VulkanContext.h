#pragma once

#include "engine/renderer/GraphicsContext.h"
#include "VulkanUtil.h"

#include <vector>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace longmarch 
{
	struct LongMarch_Vulkan_Frame
	{
		VkCommandPool       CommandPool;
		VkCommandBuffer     CommandBuffer;
		VkFence             Fence;
		VkImage             Backbuffer;
		VkImageView         BackbufferView;
		VkFramebuffer       Framebuffer;
	};

	struct LongMarch_Vulkan_FrameSemaphores
	{
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
	};

	struct LongMarch_Vulkan_Window
	{
		std::vector<LongMarch_Vulkan_Frame>		      Frames;
		std::vector<LongMarch_Vulkan_FrameSemaphores> FrameSemaphores;

		VkExtent2D		    Extent;
		VkSwapchainKHR      Swapchain;
		VkSurfaceKHR        Surface;
		VkSurfaceFormatKHR  SurfaceFormat;
		VkPresentModeKHR    PresentMode;
		VkRenderPass        RenderPass;
		VkPipeline          Pipeline;               // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
		bool                ClearEnable;
		VkClearValue        ClearValue;
		uint32_t			FrameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
		uint32_t			ImageCount;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
		uint32_t			SemaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)

		LongMarch_Vulkan_Window()
		{
			memset(this, 0, sizeof(*this));
			PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
			ClearEnable = true;
		}
	};

	class VulkanContext final : public GraphicsContext, public BaseAtomicClassStatic
	{
	private:
		// Logic device
		struct GraphicQueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			bool IsComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
		struct ComputeQueueFamilyIndices
		{
			std::optional<uint32_t> computeFamily;
			bool IsComplete()
			{
				return computeFamily.has_value();
			}
		};
		// Swap chain
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

	public:
		NONCOPYABLE(VulkanContext);
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void ResizeSwapChain(int width, int height) override; 
		virtual void* GetNativeWindow() override { return m_WindowHandle; };

	private:
		void CreateInstance();
		void SetupDebugCallback();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();

		// Logic device
		ComputeQueueFamilyIndices FindComputeQueueFamilies(const VkPhysicalDevice& device);
		GraphicQueueFamilyIndices FindGraphicQueueFamilies(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
		bool IsDeviceSuitable(const VkPhysicalDevice& device);

		// Swap chain
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device);

	private:
		int							 m_ContextID;
		GLFWwindow*					 m_WindowHandle{ nullptr };

		VkAllocationCallbacks*		 m_Allocator{ nullptr };
		VkDebugUtilsMessengerEXT	 m_DebugUtilsMessenger{ VK_NULL_HANDLE };
		VkDebugReportCallbackEXT	 m_DebugCallback{ VK_NULL_HANDLE };
									 
		VkPhysicalDevice			 m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice					 m_DeviceCompute{ VK_NULL_HANDLE };
		VkDevice					 m_DeviceGraphics{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties   m_DeviceProperties;
		VkPhysicalDeviceFeatures	 m_DeviceFeatures;
									 
		ComputeQueueFamilyIndices	 m_ComputeQueueIndices;
		GraphicQueueFamilyIndices	 m_GraphicQueueIndices;
		VkQueue						 m_ComputeQueue{ VK_NULL_HANDLE };
		VkQueue						 m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue						 m_PresentQueue{ VK_NULL_HANDLE };

		LongMarch_Vulkan_Window		 m_Vkwd;

	private:
		inline static VkInstance		 s_Instance{ VK_NULL_HANDLE };
		inline static std::atomic_int	 s_ContextID{ 0 };
		inline static std::atomic_bool	 s_init{ false };
	};
}


