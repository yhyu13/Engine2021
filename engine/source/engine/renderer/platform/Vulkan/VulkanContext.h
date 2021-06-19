#pragma once

#include "engine/renderer/GraphicsContext.h"
#include "VulkanUtil.h"

#include <vector>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace longmarch 
{
	struct Vulkan_Frame
	{
		VkCommandPool       CommandPool;
		VkCommandBuffer     CommandBuffer;
		VkFence             Fence;
		VkImage             Backbuffer;
		VkImageView         BackbufferView;
		VkFramebuffer       Framebuffer;
	};

	struct Vulkan_FrameSemaphores
	{
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
	};

	struct Vulkan_Window
	{
		std::vector<Vulkan_Frame>		      Frames;
		std::vector<Vulkan_FrameSemaphores>   FrameSemaphores;

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

		Vulkan_Window()
		{
			memset(this, 0, sizeof(*this));
			PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
			ClearEnable = true;
		}

		inline Vulkan_Frame* CurrentFrame()
		{
			return &Frames[this->FrameIndex];
		}
		inline Vulkan_FrameSemaphores* CurrentFrameSenaphore()
		{
			return &FrameSemaphores[this->SemaphoreIndex];
		}
		inline void IncrementFrameIndex()
		{
			this->FrameIndex = (this->FrameIndex + 1) % this->ImageCount;
		}
		inline void IncrementSemaphoreIndex()
		{
			this->SemaphoreIndex = (this->SemaphoreIndex + 1) % this->ImageCount;
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
		struct TransferQueueFamilyIndices
		{
			std::optional<uint32_t> transferFamily;
			bool IsComplete()
			{
				return transferFamily.has_value();
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
		virtual void RebuildSwapChain(int width = -1, int height = -1) override; 
		inline virtual void* GetNativeWindow() override { return m_WindowHandle; };

		inline Vulkan_Window* GetVulkan_Window() { return &m_Vkwd; };

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

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
		TransferQueueFamilyIndices FindTransferQueueFamilies(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
		bool IsDeviceSuitable(const VkPhysicalDevice& device);

		// Swap chain
		SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 
		uint32_t GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode);
		uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t minImageCount);

	public:

		VkAllocationCallbacks*		 m_Allocator{ nullptr };

		VkPhysicalDevice			 m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice					 m_Device{ VK_NULL_HANDLE };
		VkDescriptorPool			 m_DescriptorPool{ VK_NULL_HANDLE };
		VkQueue						 m_ComputeQueue{ VK_NULL_HANDLE };
		VkQueue						 m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue						 m_PresentQueue{ VK_NULL_HANDLE };
		ComputeQueueFamilyIndices	 m_ComputeQueueIndices;
		GraphicQueueFamilyIndices	 m_GraphicQueueIndices;
		TransferQueueFamilyIndices	 m_TransferQueueIndices;

	private:
		Vulkan_Window				 m_Vkwd;
		GLFWwindow*					 m_WindowHandle{ nullptr }; 

		VkPhysicalDeviceProperties   m_DeviceProperties;
		VkPhysicalDeviceFeatures	 m_DeviceFeatures;

		VkDebugUtilsMessengerEXT	 m_DebugUtilsMessenger{ VK_NULL_HANDLE };
		VkDebugReportCallbackEXT	 m_DebugCallback{ VK_NULL_HANDLE };
		int							 m_ContextID{ 0 };
		bool						 m_SwapChainRebuild{ false };

	public:
		inline static VkInstance		 s_Instance{ VK_NULL_HANDLE };

	private:
		inline static std::atomic_int	 s_ContextID{ 0 };
		inline static std::atomic_bool	 s_init{ false };
	};

#define GET_VK_CONTEXT() std::static_pointer_cast<VulkanContext>(Engine::GetGraphicsContext())
}