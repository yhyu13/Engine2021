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

longmarch::VulkanContext::VulkanContext(GLFWwindow* windowHandle)
{
    ASSERT(windowHandle != nullptr, "Window handle is NULL!"); 
    m_ContextID = s_ContextID++;
    m_WindowHandle = windowHandle;
    int w, h;
    glfwGetFramebufferSize(windowHandle, &w, &h);   
    m_Vkwd.Extent = VkExtent2D{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
}

longmarch::VulkanContext::~VulkanContext()
{
    for (auto& frame : m_Vkwd.Frames)
    {
        vkDestroyImageView(m_DeviceGraphics, frame.BackbufferView, m_Allocator);
    }

    vkDestroySwapchainKHR(m_DeviceGraphics, m_Vkwd.Swapchain, m_Allocator);
    vkDestroyDevice(m_DeviceGraphics, m_Allocator); 
    vkDestroyDevice(m_DeviceCompute, m_Allocator);
    vkDestroySurfaceKHR(s_Instance, m_Vkwd.Surface, m_Allocator);

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
}

void longmarch::VulkanContext::ResizeSwapChain(int width, int height)
{
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
            VK_CHECK_ERROR(CreateDebugUtilsMessengerEXT(s_Instance, &createInfo, m_Allocator, &m_DebugUtilsMessenger), "[Vulkan] failed to set up debug util messenger!");
        }
        {
            VkDebugReportCallbackCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            createInfo.pfnCallback = DebugReport;
            createInfo.pUserData = &m_ContextID;
            VK_CHECK_ERROR(CreateDebugReportCallbackEXT(s_Instance, &createInfo, m_Allocator, &m_DebugCallback), "[Vulkan] failed to set up debug report callback!");
        }
    }
}

void longmarch::VulkanContext::CreateSurface()
{
    VkSurfaceKHR rawSurface;
    VK_CHECK_ERROR(glfwCreateWindowSurface(s_Instance, m_WindowHandle, m_Allocator, &rawSurface), "[Vulkan] failed to create window surface!");
    m_Vkwd.Surface = rawSurface;
}

void longmarch::VulkanContext::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(s_Instance, &deviceCount, nullptr);
    if (deviceCount == 0) 
    {
       ENGINE_EXCEPT(L"[Vulkan] failed to find GPUs with Vulkan support!");
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
        ENGINE_EXCEPT(L"[Vulkan] failed to find a suitable GPU!");
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

        VK_CHECK_ERROR(vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_DeviceCompute), "[Vulkan] failed to create compute logical device!");
        vkGetDeviceQueue(m_DeviceCompute, m_ComputeQueueIndices.computeFamily.value(), 0, &m_ComputeQueue);
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

        VK_CHECK_ERROR(vkCreateDevice(m_PhysicalDevice, &createInfo, m_Allocator, &m_DeviceGraphics), "[Vulkan] failed to create graphics logical device!");
        vkGetDeviceQueue(m_DeviceGraphics, m_GraphicQueueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_DeviceGraphics, m_GraphicQueueIndices.presentFamily.value(), 0, &m_PresentQueue);
    }
}

void longmarch::VulkanContext::CreateSwapChain()
{
    // Swap chain details
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    // At least double buffer
    uint32_t imageCount = std::max(swapChainSupport.capabilities.minImageCount + 1u, 2u);
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Swap chain info
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Vkwd.Surface;
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
    createInfo.oldSwapchain = VkSwapchainKHR(nullptr);

    VkSwapchainKHR sw;
    VK_CHECK_ERROR(vkCreateSwapchainKHR(m_DeviceGraphics, &createInfo, m_Allocator, &sw), "[Vulkan] failed to create graphics swap chain!");
    m_Vkwd.Swapchain = sw;

    m_Vkwd.PresentMode = presentMode;
    m_Vkwd.ImageCount = imageCount;
    m_Vkwd.Extent = extent;
    m_Vkwd.SurfaceFormat = surfaceFormat;
    m_Vkwd.Frames.resize(imageCount);
    m_Vkwd.FrameSemaphores.resize(imageCount);

    VkImage backbuffers[16] = {};
    ASSERT(m_Vkwd.ImageCount >= 2u, "");
    ASSERT(m_Vkwd.ImageCount < ((int)(sizeof(backbuffers) / sizeof(*(backbuffers)))), "");
    vkGetSwapchainImagesKHR(m_DeviceGraphics, m_Vkwd.Swapchain, &m_Vkwd.ImageCount, backbuffers);

    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = m_Vkwd.SurfaceFormat.format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    info.subresourceRange = image_range;
    for (uint32_t i = 0u; i < imageCount; ++i)
    {
        auto fd = &m_Vkwd.Frames[i];
        fd->Backbuffer = backbuffers[i];
        info.image = fd->Backbuffer;
        VK_CHECK_ERROR(vkCreateImageView(m_DeviceGraphics, &info, m_Allocator, &fd->BackbufferView), "[Vulkan] faile to create image view!");
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Vkwd.Surface, &res);
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
    uint32_t extensionCount;
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
        VkExtent2D actualExtent = m_Vkwd.Extent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

longmarch::VulkanContext::SwapChainSupportDetails longmarch::VulkanContext::QuerySwapChainSupport(const VkPhysicalDevice& device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Vkwd.Surface, &details.capabilities);
    {
        uint32_t avail_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Vkwd.Surface, &avail_count, NULL);
        details.formats.resize(avail_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Vkwd.Surface, &avail_count, details.formats.data());
    }
    {
        uint32_t avail_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Vkwd.Surface, &avail_count, NULL);
        details.presentModes.resize(avail_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Vkwd.Surface, &avail_count, details.presentModes.data());
    }

    return details;
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
        ENGINE_WARN(Str("[%d]Vulkan Warning: %s", m_ContextID, pCallbackData->pMessage));
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_ERROR(Str("[%d]Vulkan Error: %s", m_ContextID, pCallbackData->pMessage));
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        ENGINE_CRITICAL(Str("[%d]Vulkan Fatal: %s", m_ContextID, pCallbackData->pMessage));
        break;
    default:
        ENGINE_EXCEPT(L"Unkown VkVkDebugUtilsMessageSeverityFlagBitsEXT!");
        break;
    }
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    if (auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"); func != nullptr)
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
    if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"); func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    int m_ContextID = *(reinterpret_cast<int*>(pUserData));
    ENGINE_DEBUG(Str("[%d]Vulkan Debug report from ObjectType: %i. Message: %s\n", m_ContextID, objectType, pMessage));
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
    uint32_t layerCount;
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
    uint32_t extensionCount;
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