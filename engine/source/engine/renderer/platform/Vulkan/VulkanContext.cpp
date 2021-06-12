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
const std::vector<const char*> VkDeviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Validation layer
VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

bool CheckRequiredValidationLayerSupport();
std::vector<const char*> GetRequiredExtensions();

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
    auto wd = &m_Vkwd;
    DestroyVulkan_Window(s_Instance, m_GraphicsDevice, wd, m_Allocator);
    vkDestroyDevice(m_GraphicsDevice, m_Allocator); 
    vkDeviceWaitIdle(m_ComputeDevice);
    vkDestroyDevice(m_ComputeDevice, m_Allocator);
    vkDestroySurfaceKHR(s_Instance, wd->Surface, m_Allocator);

    if constexpr (VkEnableValidationLayer) 
    {
        DestroyDebugUtilsMessengerEXT(s_Instance, m_DebugUtilsMessenger, m_Allocator);
        DestroyDebugReportCallbackEXT(s_Instance, m_DebugCallback, m_Allocator);
    }

    if (--s_ContextID == 0)
    {
        LOCK_GUARD_S();
        s_init = false;
        vkDestroyInstance(s_Instance, m_Allocator);
        s_Instance = VK_NULL_HANDLE;
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
    ASSERT(result == VK_SUCCESS, Str("[%d][Vulkan] Swap chain failed!", m_ContextID));
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

VkCommandBuffer longmarch::VulkanContext::BeginSingleTimeGraphicsCommands()
{
    auto wd = &m_Vkwd;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = wd->CurrentFrame()->CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_GraphicsDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void longmarch::VulkanContext::EndSingleTimeGraphicsCommands(VkCommandBuffer commandBuffer)
{
    auto wd = &m_Vkwd;
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    vkFreeCommandBuffers(m_GraphicsDevice, wd->CurrentFrame()->CommandPool, 1, &commandBuffer);
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

        VK_CHECK_ERROR(vkCreateInstance(&createInfo, m_Allocator, &s_Instance), "[Vulkan] failed to create instance!");
    }
}

void longmarch::VulkanContext::SetupDebugCallback()
{
    if constexpr (VkEnableValidationLayer)
    {
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = DebugUtilsMessenger;
            createInfo.pUserData = &m_ContextID;
            VK_CHECK_ERROR(CreateDebugUtilsMessengerEXT(s_Instance, &createInfo, m_Allocator, &m_DebugUtilsMessenger), Str("[%d][Vulkan] failed to set up debug util messenger!", m_ContextID));
        }
        {
            VkDebugReportCallbackCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            createInfo.pfnCallback = DebugReport;
            createInfo.pUserData = &m_ContextID;
            VK_CHECK_ERROR(CreateDebugReportCallbackEXT(s_Instance, &createInfo, m_Allocator, &m_DebugCallback), Str("[%d][Vulkan] failed to set up debug report callback!", m_ContextID));
        }
    }
}

void longmarch::VulkanContext::CreateSurface()
{
    auto wd = &m_Vkwd;
    VK_CHECK_ERROR(glfwCreateWindowSurface(s_Instance, m_WindowHandle, m_Allocator, &wd->Surface), Str("[%d][Vulkan] failed to create window surface!", m_ContextID));
}

void longmarch::VulkanContext::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(s_Instance, &deviceCount, nullptr);
    if (deviceCount == 0) 
    {
       ENGINE_EXCEPT(wStr(L"[%d][Vulkan] failed to find GPUs with Vulkan support!", m_ContextID));
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(s_Instance, &deviceCount, devices.data());

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
    {
        // Queue info
        const float queuePriority[] = { 1.0f };
        VkDeviceQueueCreateInfo queueInfo[1] = {};
        queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[0].queueFamilyIndex = m_ComputeQueueIndices.computeFamily.value();
        queueInfo[0].queueCount = 1;
        queueInfo[0].pQueuePriorities = queuePriority;

        // Device info
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
        createInfo.pQueueCreateInfos = queueInfo;
        createInfo.enabledExtensionCount = 0;

        // Device feature
        auto deviceFeatures = VkPhysicalDeviceFeatures(); // TODO: assaign feature on need
        createInfo.pEnabledFeatures = &deviceFeatures;
        
        // Device layer
        if constexpr (VkEnableValidationLayer)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedLayerNames.size());
            createInfo.ppEnabledLayerNames = VkRequestedLayerNames.data();
        }

        VK_CHECK_ERROR(vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_ComputeDevice), Str("[%d][Vulkan] failed to create compute logical device!", m_ContextID));
        vkGetDeviceQueue(m_ComputeDevice, m_ComputeQueueIndices.computeFamily.value(), 0, &m_ComputeQueue);

        VkDescriptorPoolSize pool_sizes[] =
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
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * (sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.poolSizeCount = static_cast<uint32_t>((sizeof(pool_sizes) / sizeof(pool_sizes[0])));
        pool_info.pPoolSizes = pool_sizes;
        VK_CHECK_ERROR(vkCreateDescriptorPool(m_ComputeDevice, &pool_info, m_Allocator, &m_ComputeDescriptorPool), Str("[%d][Vulkan] failed to create graphics descriptor pool!", m_ContextID));
    }
    // Graphics device
    {
        // Queue info
        const float queuePriority[] = { 1.0f };
        VkDeviceQueueCreateInfo queueInfo[1] = {};
        queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[0].queueFamilyIndex = m_GraphicQueueIndices.graphicsFamily.value();
        queueInfo[0].queueCount = 1;
        queueInfo[0].pQueuePriorities = queuePriority;

        // Device info
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
        createInfo.pQueueCreateInfos = queueInfo;
        
        // Device extension
        createInfo.enabledExtensionCount = static_cast<uint32_t>(VkDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = VkDeviceExtensions.data();

        // Device feature
        auto deviceFeatures = VkPhysicalDeviceFeatures(); // TODO: assaign feature on need
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Device layer
        if constexpr (VkEnableValidationLayer)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedLayerNames.size());
            createInfo.ppEnabledLayerNames = VkRequestedLayerNames.data();
        }

        VK_CHECK_ERROR(vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_GraphicsDevice), Str("[%d][Vulkan] failed to create graphics logical device!", m_ContextID));
        vkGetDeviceQueue(m_GraphicsDevice, m_GraphicQueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_GraphicsDevice, m_GraphicQueueIndices.presentFamily.value(), 0, &m_PresentQueue);

        VkDescriptorPoolSize pool_sizes[] =
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
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * (sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.poolSizeCount = static_cast<uint32_t>((sizeof(pool_sizes) / sizeof(pool_sizes[0])));
        pool_info.pPoolSizes = pool_sizes;
        VK_CHECK_ERROR(vkCreateDescriptorPool(m_GraphicsDevice, &pool_info, m_Allocator, &m_GraphicsDescriptorPool), Str("[%d][Vulkan] failed to create graphics descriptor pool!", m_ContextID));
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
    VK_CHECK_ERROR(vkDeviceWaitIdle(m_GraphicsDevice), Str("[%d][Vulkan] divce wait error!", m_ContextID));

    // Destroy old Framebuffer
    for (uint32_t i = 0; i < wd->ImageCount; ++i)
    {
        DestroyVulakn_Frame(m_GraphicsDevice, &wd->Frames[i], m_Allocator);
        DestroyVulkan_FrameSemaphores(m_GraphicsDevice, &wd->FrameSemaphores[i], m_Allocator);
    }
    wd->Frames.clear();
    wd->FrameSemaphores.clear();
    wd->ImageCount = 0;
    if (wd->RenderPass)
    {
        vkDestroyRenderPass(m_GraphicsDevice, wd->RenderPass, m_Allocator);
    }
    if (wd->Pipeline)
    {
        vkDestroyPipeline(m_GraphicsDevice, wd->Pipeline, m_Allocator);
    }

    // Swap chain info
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

    // Create swap chain
    VK_CHECK_ERROR(vkCreateSwapchainKHR(m_GraphicsDevice, &createInfo, m_Allocator, &wd->Swapchain), Str("[%d][Vulkan] failed to create graphics swap chain!", m_ContextID));

    // Fill swap chain struct
    wd->PresentMode = presentMode;
    wd->ImageCount = imageCount;
    wd->Extent = extent;
    wd->SurfaceFormat = surfaceFormat;
    wd->Frames.resize(imageCount);
    wd->FrameSemaphores.resize(imageCount);

    // Get image buffer
    std::vector<VkImage> backbuffers(wd->ImageCount);
    VK_CHECK_ERROR(vkGetSwapchainImagesKHR(m_GraphicsDevice, wd->Swapchain, &wd->ImageCount, backbuffers.data()), Str("[%d][Vulkan] failed to get images!", m_ContextID));
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        auto fd = &wd->Frames[i];
        fd->Backbuffer = backbuffers[i];
    }

    // Destroy old swap chain if exists
    if (oldSwapchain)
    {
        vkDestroySwapchainKHR(m_GraphicsDevice, oldSwapchain, m_Allocator);
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
        VK_CHECK_ERROR(vkCreateImageView(m_GraphicsDevice, &info, m_Allocator, &fd->BackbufferView), Str("[%d][Vulkan] failed to create image view!", m_ContextID));
    }

    // Create render pass
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

    VK_CHECK_ERROR(vkCreateRenderPass(m_GraphicsDevice, &renderPassInfo, m_Allocator, &wd->RenderPass), Str("[%d][Vulkan] failed to create render pass!", m_ContextID));

    // Create frame buffer
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
        VK_CHECK_ERROR(vkCreateFramebuffer(m_GraphicsDevice, &frameBufferInfo, m_Allocator, &fd->Framebuffer), Str("[%d][Vulkan] failed to create frame buffer!", m_ContextID));
    }
}

bool longmarch::VulkanContext::IsDeviceSuitable(const VkPhysicalDevice& device)
{
    m_ComputeQueueIndices = FindComputeQueueFamilies(device);
    m_GraphicQueueIndices = FindGraphicQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) 
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vkGetPhysicalDeviceProperties(device, &m_DeviceProperties);
    vkGetPhysicalDeviceFeatures(device, &m_DeviceFeatures);

    return m_DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                m_DeviceFeatures.geometryShader &&
                m_DeviceFeatures.multiDrawIndirect &&
                extensionsSupported &&
                swapChainAdequate &&
                m_ComputeQueueIndices.IsComplete() &&
                m_GraphicQueueIndices.IsComplete();
}

longmarch::VulkanContext::ComputeQueueFamilyIndices longmarch::VulkanContext::FindComputeQueueFamilies(const VkPhysicalDevice& device)
{
    ComputeQueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    bool found = false;
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.computeFamily = i;
            }
        }
        if (indices.IsComplete())
        {
            found = true;
            break;
        }
        ++i;
    }

    if (!found)
    {
        ENGINE_EXCEPT(L"Couldn't find required queue family!");
    }
    return indices;
}

longmarch::VulkanContext::GraphicQueueFamilyIndices longmarch::VulkanContext::FindGraphicQueueFamilies(const VkPhysicalDevice& device)
{
    auto wd = &m_Vkwd;
    GraphicQueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    bool found = false;
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            VkBool32 res; 
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, wd->Surface, &res);
            if (res == VK_TRUE)
            {
                indices.presentFamily = i;
            }
        }
        if (indices.IsComplete())
        {
            found = true;
            break;
        }
        ++i;
    }

    if (!found)
    {
        ENGINE_EXCEPT(L"Couldn't find required queue family!");
    }
    return indices;
}

bool longmarch::VulkanContext::CheckDeviceExtensionSupport(const VkPhysicalDevice& device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

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
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, wd->Surface, &details.capabilities);
    }
    {
        uint32_t avail_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, wd->Surface, &avail_count, NULL);
        details.formats.resize(avail_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, wd->Surface, &avail_count, details.formats.data());
    }
    {
        uint32_t avail_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, wd->Surface, &avail_count, NULL);
        details.presentModes.resize(avail_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, wd->Surface, &avail_count, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR longmarch::VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
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
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : availablePresentModes) 
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            bestMode = availablePresentMode;
        }
    }
    return bestMode;
}

VkExtent2D longmarch::VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
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
        return 3;
    if (present_mode == VK_PRESENT_MODE_FIFO_KHR || present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
        return 2;
    if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        return 1;
    return 1;
}

uint32_t longmarch::VulkanContext::ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t minImageCount)
{
    // At least double buffer
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
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
    vkEnumerateInstanceExtensionProperties(nullptr , &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtension(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtension.data());

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
    vkDeviceWaitIdle(device);

    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        DestroyVulakn_Frame(device, &wd->Frames[i], allocator);
        DestroyVulkan_FrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    }
    wd->Frames.clear();
    wd->FrameSemaphores.clear();
    vkDestroyPipeline(device, wd->Pipeline, allocator);
    vkDestroyRenderPass(device, wd->RenderPass, allocator);
    vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
    vkDestroySurfaceKHR(instance, wd->Surface, allocator);

    *wd = Vulkan_Window();
}