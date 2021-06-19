#include "engine-precompiled-header.h"
#include "VulkanContext.h"

// Validation layer
constexpr bool VkEnableValidationLayer = 
{
#ifndef _SHIPPING
    true
#else
    false
#endif // _SHIPPING
};
// Remember to launch the vkconfig.exe under C:\VulkanSDK\1.2.x\Tools: 
// 1. check the checkbox of change to layer to be consistent
// 2. click the select layer button and set VK_LAYER_KHRONOS_validation to be "Forced On"
const std::vector<const char*> VkRequestedLayerNames = 
{
    "VK_LAYER_KHRONOS_validation"
};
// Instance extensions are returned by glfw, see GetRequiredExtensions() function below
//const std::vector<const char*> VkInstanceExtensions =
//{
//    VK_KHR_SURFACE_EXTENSION_NAME
//};
const std::vector<const char*> VkDeviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Vulkan window application required extension
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, // Vulkan ray tracing required extension
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, // Vulkan ray tracing required extension
};

// Validation layer
VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

bool CheckRequiredValidationLayerSupport();

// Extensions
std::vector<const char*> GetRequiredExtensions();

// Helper functions
void DestroyVulakn_Frame(VkDevice device, Vulkan_Frame* fd, const VkAllocationCallbacks* allocator);
void DestroyVulkan_FrameSemaphores(VkDevice device, Vulkan_FrameSemaphores* fsd, const VkAllocationCallbacks* allocator);
void DestroyVulkan_Window(VkInstance instance, VkDevice device, Vulkan_Window* wd, const VkAllocationCallbacks* allocator);

longmarch::VulkanContext::VulkanContext(GLFWwindow* windowHandle)
    :
    m_WindowHandle(windowHandle)
{
    ASSERT(windowHandle != nullptr, "Window handle is NULL!");
    m_ContextID = s_ContextID++;

    int width, height;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    auto wd = &m_Vkwd;
    wd->Extent.width = width;
    wd->Extent.height = height;
};

longmarch::VulkanContext::~VulkanContext()
{
    if (s_Instance)
    {
        auto wd = &m_Vkwd;
        if (m_Device)
        {
            DestroyVulkan_Window(s_Instance, m_Device, wd, m_Allocator);
            vkDestroyDevice(m_Device, m_Allocator);
        }
        vkDestroySurfaceKHR(s_Instance, wd->Surface, m_Allocator);

        if constexpr (VkEnableValidationLayer)
        {
            DestroyDebugUtilsMessengerEXT(s_Instance, m_DebugUtilsMessenger, m_Allocator);
            DestroyDebugReportCallbackEXT(s_Instance, m_DebugCallback, m_Allocator);
        }
    }

    if (--s_ContextID == 0)
    {
        LOCK_GUARD_S();
        s_init = false;
        if (s_Instance)
        {
            vkDestroyInstance(s_Instance, m_Allocator);
            s_Instance = VK_NULL_HANDLE;
        }
    }
}

void longmarch::VulkanContext::Init()
{
    CreateInstance();
    SetupDebugCallback();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
}
 
void longmarch::VulkanContext::SwapBuffers()
{
    if (m_SwapChainRebuild)
    {
        CreateSwapChain();
    }
    auto wd = &m_Vkwd;
    VkSemaphore render_complete_semaphore = wd->CurrentFrameSenaphore()->RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult result = vkQueuePresentKHR(m_GraphicsQueue, &info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RebuildSwapChain(-1, -1);
        return;
    }
    VK_CHECK_RESULT(result, Str("[%d][Vulkan] Swap chain failed!", m_ContextID));
    wd->IncrementFrameIndex();
    wd->IncrementSemaphoreIndex();
}

void longmarch::VulkanContext::RebuildSwapChain(int width, int height)
{
    auto wd = &m_Vkwd;
    if (width == -1 && height == -1)
    {
        glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    }
    wd->Extent.width = width;
    wd->Extent.height = height;
    m_SwapChainRebuild = true;
}

VkCommandBuffer longmarch::VulkanContext::BeginSingleTimeCommands()
{
    auto wd = &m_Vkwd;

    // Allocate command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = wd->CurrentFrame()->CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer));

    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    return commandBuffer;
}

void longmarch::VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    auto wd = &m_Vkwd;

    // End command buffer recording
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    // Prepare submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FLAGS_NONE;
    VkFence fence;
    VK_CHECK_RESULT(vkCreateFence(m_Device, &fenceCreateInfo, m_Allocator, &fence));
    // Submit commands to queue
    VK_CHECK_RESULT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence));
    // Wait for the fence to signal that command buffer has finished executing
    VK_CHECK_RESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, VK_DEFAULT_FENCE_TIMEOUT));
    vkDestroyFence(m_Device, fence, m_Allocator);

    // Free allocated command buffer
    vkFreeCommandBuffers(m_Device, wd->CurrentFrame()->CommandPool, 1, &commandBuffer);
}

void longmarch::VulkanContext::CreateInstance()
{
    if (!s_init)
    {
        LOCK_GUARD_S();
        s_init = true;

        // Setup Vulkan
        if (!glfwVulkanSupported())
        {
            ENGINE_EXCEPT(L"GLFW: Vulkan Not Supported!");
            return;
        }

        ENGINE_INFO("[Vulkan] Info: ");
        if constexpr (VkEnableValidationLayer)
        {
            if (!CheckRequiredValidationLayerSupport())
            {
                ENGINE_EXCEPT(L"[Vulkan] validation layers requested, but not all are available!");
            }
        }

        // App info
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "LongMarch App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "LongMarch Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        // Instance create info
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Instance extension
        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Instance layer
        if constexpr (VkEnableValidationLayer) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedLayerNames.size());
            createInfo.ppEnabledLayerNames = VkRequestedLayerNames.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        VK_CHECK_RESULT(vkCreateInstance(&createInfo, m_Allocator, &s_Instance), "[Vulkan] failed to create instance!");
    }
    else
    {
        ENGINE_EXCEPT(L"Multiple vulkan instances are not handled!");
    }
}

void longmarch::VulkanContext::SetupDebugCallback()
{
    if constexpr (VkEnableValidationLayer)
    {
        {
            // Enbale vulkan logging system (with different level of log - debug, info, warning, error, critical)
            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = DebugUtilsMessenger;
            createInfo.pUserData = &m_ContextID;
            VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(s_Instance, &createInfo, m_Allocator, &m_DebugUtilsMessenger), Str("[%d][Vulkan] failed to set up debug util messenger!", m_ContextID));
        }
        {
            // Enable vulkan debug report system (usually would just repeat what the logging system says in a debugging report format)
            VkDebugReportCallbackCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            createInfo.pfnCallback = DebugReport;
            createInfo.pUserData = &m_ContextID;
            VK_CHECK_RESULT(CreateDebugReportCallbackEXT(s_Instance, &createInfo, m_Allocator, &m_DebugCallback), Str("[%d][Vulkan] failed to set up debug report callback!", m_ContextID));
        }
    }
}

void longmarch::VulkanContext::CreateSurface()
{
    auto wd = &m_Vkwd;
    // Use glfw WIS function to create window surface
    VK_CHECK_RESULT(glfwCreateWindowSurface(s_Instance, m_WindowHandle, m_Allocator, &wd->Surface), Str("[%d][Vulkan] failed to create window surface!", m_ContextID));
}

void longmarch::VulkanContext::PickPhysicalDevice()
{
    // Get all available physical devices
    uint32_t deviceCount = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(s_Instance, &deviceCount, nullptr));
    if (deviceCount == 0) 
    {
       ENGINE_EXCEPT(wStr(L"[%d][Vulkan] failed to find GPUs with Vulkan support!", m_ContextID));
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(s_Instance, &deviceCount, devices.data()));

    // Select the first suitable device
    for (const auto& device : devices) 
    {
        if (IsDeviceSuitable(device)) 
        {
            m_PhysicalDevice = device;
            break;
        }
    }
    if (m_PhysicalDevice == VK_NULL_HANDLE)
    {
        ENGINE_EXCEPT(wStr(L"[%d][Vulkan] failed to find a suitable GPU!", m_ContextID));
    }
}

void longmarch::VulkanContext::CreateLogicalDevice()
{
    // Compute device
    // Graphics device
    {
        // Queue info
        const float queuePriority[] = { 1.0f };
        VkDeviceQueueCreateInfo queueInfo[3] = {};
        queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[0].queueFamilyIndex = m_GraphicQueueIndices.graphicsFamily.value();
        queueInfo[0].queueCount = 1;
        queueInfo[0].pQueuePriorities = queuePriority;
        queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[1].queueFamilyIndex = m_ComputeQueueIndices.computeFamily.value();
        queueInfo[1].queueCount = 1;
        queueInfo[1].pQueuePriorities = queuePriority;
        queueInfo[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[2].queueFamilyIndex = m_TransferQueueIndices.transferFamily.value();
        queueInfo[2].queueCount = 1;
        queueInfo[2].pQueuePriorities = queuePriority;

        // Device info
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(sizeof(queueInfo) / sizeof(queueInfo[0]));
        createInfo.pQueueCreateInfos = queueInfo;
        
        // Device extension
        createInfo.enabledExtensionCount = static_cast<uint32_t>(VkDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = VkDeviceExtensions.data();

        // Device feature
        auto deviceFeatures = VkPhysicalDeviceFeatures();
        deviceFeatures.geometryShader = true;
        deviceFeatures.multiDrawIndirect = true;
        deviceFeatures.depthClamp = true;
        deviceFeatures.samplerAnisotropy = true;
        deviceFeatures.sparseBinding = true;
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Device layer
        if constexpr (VkEnableValidationLayer)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedLayerNames.size());
            createInfo.ppEnabledLayerNames = VkRequestedLayerNames.data();
        }

        VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_Device), Str("[%d][Vulkan] failed to create graphics logical device!", m_ContextID));
        vkGetDeviceQueue(m_Device, m_ComputeQueueIndices.computeFamily.value(), 0, &m_ComputeQueue);
        vkGetDeviceQueue(m_Device, m_GraphicQueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_GraphicQueueIndices.presentFamily.value(), 0, &m_PresentQueue);

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 },
            { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1000},
        };
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = static_cast<uint32_t>(1000 * (sizeof(poolSizes) / sizeof(poolSizes[0])));
        poolInfo.poolSizeCount = static_cast<uint32_t>((sizeof(poolSizes) / sizeof(poolSizes[0])));
        poolInfo.pPoolSizes = poolSizes;
        VK_CHECK_RESULT(vkCreateDescriptorPool(m_Device, &poolInfo, m_Allocator, &m_DescriptorPool), Str("[%d][Vulkan] failed to create graphics descriptor pool!", m_ContextID));
    }
}

void longmarch::VulkanContext::CreateSwapChain()
{
    m_SwapChainRebuild = false;

    // Query swap chain
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = ChooseImageCount(swapChainSupport.capabilities, GetMinImageCountFromPresentMode(presentMode));
    
    auto wd = &m_Vkwd;
    VkSwapchainKHR oldSwapchain = wd->Swapchain;
    wd->Swapchain = VK_NULL_HANDLE;

    // Wait until device is idle
    VK_CHECK_RESULT(vkDeviceWaitIdle(m_Device));

    // Destroy old Framebuffer
    for (uint32_t i = 0; i < wd->ImageCount; ++i)
    {
        DestroyVulakn_Frame(m_Device, &wd->Frames[i], m_Allocator);
        DestroyVulkan_FrameSemaphores(m_Device, &wd->FrameSemaphores[i], m_Allocator);
    }
    wd->Frames.clear();
    wd->FrameSemaphores.clear();
    wd->ImageCount = 0;
    if (wd->RenderPass)
    {
        vkDestroyRenderPass(m_Device, wd->RenderPass, m_Allocator);
    }
    if (wd->Pipeline)
    {
        vkDestroyPipeline(m_Device, wd->Pipeline, m_Allocator);
    }

    // Create swap chain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = wd->Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageExtent = extent;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (m_GraphicQueueIndices.graphicsFamily != m_GraphicQueueIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = { m_GraphicQueueIndices.graphicsFamily.value(), m_GraphicQueueIndices.presentFamily.value() };
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;
    VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Device, &createInfo, m_Allocator, &wd->Swapchain), Str("[%d][Vulkan] failed to create graphics swap chain!", m_ContextID));

    // Fill vulkan window struct
    wd->PresentMode = presentMode;
    wd->ImageCount = imageCount;
    wd->Extent = extent;
    wd->SurfaceFormat = surfaceFormat;
    wd->Frames.resize(imageCount);
    wd->FrameSemaphores.resize(imageCount);

    // Get image buffer
    std::vector<VkImage> backbuffers(wd->ImageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_Device, wd->Swapchain, &wd->ImageCount, backbuffers.data()), Str("[%d][Vulkan] failed to get images!", m_ContextID));
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        auto fd = &wd->Frames[i];
        fd->Backbuffer = backbuffers[i];
    }

    // Destroy old swap chain if exists
    if (oldSwapchain)
    {
        vkDestroySwapchainKHR(m_Device, oldSwapchain, m_Allocator);
    }

    // Create image buffer view
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = wd->SurfaceFormat.format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    info.subresourceRange = image_range;
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        auto fd = &wd->Frames[i];
        info.image = fd->Backbuffer;
        VK_CHECK_RESULT(vkCreateImageView(m_Device, &info, m_Allocator, &fd->BackbufferView), Str("[%d][Vulkan] failed to create image view!", m_ContextID));
    }

    // Create render pass (with a single color attachment only)
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = wd->SurfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.pDependencies = &dependency;
    VK_CHECK_RESULT(vkCreateRenderPass(m_Device, &renderPassInfo, m_Allocator, &wd->RenderPass), Str("[%d][Vulkan] failed to create render pass!", m_ContextID));

    // Create frame buffer (with a single color attachment only)
    VkImageView attachment[1];
    VkFramebufferCreateInfo frameBufferInfo = {};
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.renderPass = wd->RenderPass;
    frameBufferInfo.attachmentCount = 1;
    frameBufferInfo.pAttachments = attachment;
    frameBufferInfo.width = wd->Extent.width;
    frameBufferInfo.height = wd->Extent.height;
    frameBufferInfo.layers = 1;
    for (uint32_t i = 0; i < wd->ImageCount; ++i)
    {
        auto fd = &wd->Frames[i];
        attachment[0] = fd->BackbufferView;
        VK_CHECK_RESULT(vkCreateFramebuffer(m_Device, &frameBufferInfo, m_Allocator, &fd->Framebuffer), Str("[%d][Vulkan] failed to create frame buffer!", m_ContextID));
    }

    // Create Command Buffers
    for (uint32_t i = 0; i < wd->ImageCount; ++i)
    {
        auto fd = &wd->Frames[i];
        auto fsd = &wd->FrameSemaphores[i];
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = m_GraphicQueueIndices.graphicsFamily.value();
            VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &info, m_Allocator, &fd->CommandPool));
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->CommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &info, &fd->CommandBuffer));
        }
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VK_CHECK_RESULT(vkCreateFence(m_Device, &info, m_Allocator, &fd->Fence));
        }
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &info, m_Allocator, &fsd->ImageAcquiredSemaphore));
            VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &info, m_Allocator, &fsd->RenderCompleteSemaphore));
        }
    }
}

bool longmarch::VulkanContext::IsDeviceSuitable(const VkPhysicalDevice& device)
{
    // Find queue index
    m_ComputeQueueIndices = FindComputeQueueFamilies(device);
    m_GraphicQueueIndices = FindGraphicQueueFamilies(device);
    m_TransferQueueIndices = FindTransferQueueFamilies(device);

    bool hasCompute = m_ComputeQueueIndices.IsComplete();
    bool hasGraphics = m_GraphicQueueIndices.IsComplete();
    bool hasTransfer = m_TransferQueueIndices.IsComplete();

    // Check exntension support
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    // Check swap chain support
    bool swapChainAdequate = false;
    if (extensionsSupported) 
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    // Check device property and feature
    vkGetPhysicalDeviceProperties(device, &m_DeviceProperties);
    vkGetPhysicalDeviceFeatures(device, &m_DeviceFeatures);

    bool propertySupported = m_DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    bool featureSupported = m_DeviceFeatures.geometryShader &&
                            m_DeviceFeatures.multiDrawIndirect;

    return propertySupported &&
           featureSupported &&
           extensionsSupported &&
           swapChainAdequate &&
           hasCompute &&
           hasGraphics &&
           hasTransfer;
}

longmarch::VulkanContext::ComputeQueueFamilyIndices longmarch::VulkanContext::FindComputeQueueFamilies(const VkPhysicalDevice& device)
{
    ComputeQueueFamilyIndices indices;

    // Get all queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            // Better find a compute only queue that does not perform any graphics
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.computeFamily = i;
            }
            if (indices.IsComplete())
            {
                break;
            }
            ++i;
        }
    }
    return indices;
}

longmarch::VulkanContext::GraphicQueueFamilyIndices longmarch::VulkanContext::FindGraphicQueueFamilies(const VkPhysicalDevice& device)
{
    GraphicQueueFamilyIndices indices;
    auto wd = &m_Vkwd;

    // Get all queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            // Check for WSI support.
            // Notice that the graphics queue might be different than the present queue, we need to handle that case on creating the swap chain
            VkBool32 res; 
            VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, wd->Surface, &res));
            if (res == VK_TRUE)
            {
                indices.presentFamily = i;
            }
            if (indices.IsComplete())
            {
                break;
            }
            ++i;
        }
    }
    return indices;
}

longmarch::VulkanContext::TransferQueueFamilyIndices longmarch::VulkanContext::FindTransferQueueFamilies(const VkPhysicalDevice& device)
{
    TransferQueueFamilyIndices indices;

    // Get all queues
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            // Better find a transfer only queue that does not perform any graphics or compute
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.transferFamily = i;
            }
            if (indices.IsComplete())
            {
                break;
            }
            ++i;
        }
    }
    return indices;
}

bool longmarch::VulkanContext::CheckDeviceExtensionSupport(const VkPhysicalDevice& device)
{
    // Get all device extensions
    uint32_t extensionCount = 0;
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

    // Check if all extensions are met
    std::set<std::string> requiredExtensions(VkDeviceExtensions.begin(), VkDeviceExtensions.end());
    for (const auto& extension : availableExtensions) 
    {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

longmarch::VulkanContext::SwapChainSupportDetails longmarch::VulkanContext::QuerySwapChainSupport(const VkPhysicalDevice& device)
{
    auto wd = &m_Vkwd;
    SwapChainSupportDetails details;
    {
        // Get swap chain capacity
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, wd->Surface, &details.capabilities));
    }
    {
        // Get swap chain supported surface format
        uint32_t avail_count = 0;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, wd->Surface, &avail_count, NULL));
        details.formats.resize(avail_count);
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, wd->Surface, &avail_count, details.formats.data()));
    }
    {
        // Get swap chain present mode
        uint32_t avail_count = 0;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, wd->Surface, &avail_count, NULL));
        details.presentModes.resize(avail_count);
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, wd->Surface, &avail_count, details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR longmarch::VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
    // Prefer RGBA8 srgb format
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    }
    for (const auto& availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR longmarch::VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) 
{
    auto begin = availablePresentModes.begin();
    auto end = availablePresentModes.end();
    if (std::find(begin, end, VK_PRESENT_MODE_MAILBOX_KHR) != end)
    {
        return VK_PRESENT_MODE_MAILBOX_KHR; // Enable triple buffering
    }
    else if (std::find(begin, end, VK_PRESENT_MODE_FIFO_KHR) != end)
    {
        return VK_PRESENT_MODE_FIFO_KHR; // Enable double buffering
    }
    else if (std::find(begin, end, VK_PRESENT_MODE_IMMEDIATE_KHR) != end)
    {
        return VK_PRESENT_MODE_IMMEDIATE_KHR; // Enable single buffering
    }
    else
    {
        ENGINE_EXCEPT(wStr(L"[%d][Vulkan] No desired present mode found!", m_ContextID));
        return VK_PRESENT_MODE_MAX_ENUM_KHR;
    }
}

VkExtent2D longmarch::VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        DEBUG_PRINT(Str("[%d][Vulkan] Use cap.currentExtent width:%d, height:%d!", m_ContextID, capabilities.currentExtent.width, capabilities.currentExtent.height));
        return capabilities.currentExtent;
    }
    else 
    {
        auto wd = &m_Vkwd;
        VkExtent2D actualExtent = wd->Extent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

uint32_t longmarch::VulkanContext::GetMinImageCountFromPresentMode(VkPresentModeKHR present_mode)
{
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
        return 3; // Enable triple buffering
    }
    else if (present_mode == VK_PRESENT_MODE_FIFO_KHR || present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
    {
        return 2; // Enable double buffering
    }
    else if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    {
        return 1;  // Enable single buffering
    }
    else
    {
        ENGINE_EXCEPT(wStr(L"[%d][Vulkan] Unhandled present mode!", m_ContextID));
        return 0;
    }
}

uint32_t longmarch::VulkanContext::ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t minImageCount)
{
    // Choose a valid swap chain image count
    uint32_t imageCount = std::max(capabilities.minImageCount + 1u, minImageCount);
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }
    return imageCount;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    int m_ContextID = *(reinterpret_cast<int*>(pUserData));
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ENGINE_WARN(Str("[%d][Vulkan] Warning: %s", m_ContextID, pCallbackData->pMessage));
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_ERROR(Str("[%d][Vulkan] Error: %s", m_ContextID, pCallbackData->pMessage));
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        ENGINE_CRITICAL(Str("[%d][Vulkan]Fatal: %s", m_ContextID, pCallbackData->pMessage));
        break;
    default:
        ENGINE_EXCEPT(L"Unkown VkVkDebugUtilsMessageSeverityFlagBitsEXT!");
        break;
    }
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    if (auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"); 
        func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
    if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"); 
        func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    int m_ContextID = *(reinterpret_cast<int*>(pUserData));
    ENGINE_DEBUG(Str("[%d][Vulkan] Debug report from ObjectType: %i. Message: %s\n", m_ContextID, objectType, pMessage));
    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    if (auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"); 
        func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    if (auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"); 
        func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}

bool CheckRequiredValidationLayerSupport()
{
    // Get all available layers
    uint32_t layerCount = 0;
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
    std::vector<VkLayerProperties> availableLayers(layerCount);
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    // Print all available layers
    ENGINE_INFO("[Vulkan] Available Layers:");
    for (const auto& layerProperties : availableLayers)
    {
        ENGINE_INFO("\t{0}", layerProperties.layerName);
    }

    // Find requested layers
    for (const char* layerName : VkRequestedLayerNames) 
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                ENGINE_INFO("[Vulkan] Layer resquested {0} is found!", layerName);
                break;
            }
        }
        if (!layerFound) 
        {
            ENGINE_WARN("[Vulkan] Layer resquested {0} is not found!", layerName);
            return false;
        }
    }
    return true;
}

std::vector<const char*> GetRequiredExtensions() 
{
    uint32_t extensionCount = 0;
    VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr , &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableExtension(extensionCount);
    VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtension.data()));

    ENGINE_INFO("[Vulkan] Available Extensions:");
    for (const auto& extension : availableExtension)
    {
        ENGINE_INFO("\t{0}", extension.extensionName);
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if constexpr (VkEnableValidationLayer)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    ENGINE_INFO("[Vulkan] Requested Extensions:");
    for (const auto& extension : extensions)
    {
        ENGINE_INFO("\t{0}", extension);
    }
    return extensions;
}

void DestroyVulakn_Frame(VkDevice device, Vulkan_Frame* fd, const VkAllocationCallbacks* allocator)
{
    vkDestroyFence(device, fd->Fence, allocator);
    vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
    vkDestroyCommandPool(device, fd->CommandPool, allocator);
    fd->Fence = VK_NULL_HANDLE;
    fd->CommandBuffer = VK_NULL_HANDLE;
    fd->CommandPool = VK_NULL_HANDLE;

    vkDestroyImageView(device, fd->BackbufferView, allocator);
    vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
}

void DestroyVulkan_FrameSemaphores(VkDevice device, Vulkan_FrameSemaphores* fsd, const VkAllocationCallbacks* allocator)
{
    vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
    vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
    fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
}

void DestroyVulkan_Window(VkInstance instance, VkDevice device, Vulkan_Window* wd, const VkAllocationCallbacks* allocator)
{
    VK_CHECK_RESULT(vkDeviceWaitIdle(device));

    for (uint32_t i = 0; i < wd->ImageCount; ++i)
    {
        DestroyVulakn_Frame(device, &wd->Frames[i], allocator);
        DestroyVulkan_FrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    }
    wd->Frames.clear();
    wd->FrameSemaphores.clear();
    vkDestroyRenderPass(device, wd->RenderPass, allocator);
    if (wd->Pipeline)
    {
        vkDestroyPipeline(device, wd->Pipeline, allocator);
    }
    vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
    vkDestroySurfaceKHR(instance, wd->Surface, allocator);

    *wd = Vulkan_Window();
}