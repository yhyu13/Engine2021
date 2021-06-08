#include "engine-precompiled-header.h"
#include "VulkanContext.h"

// Validation layer
constexpr bool VkEnableValidationLayer {
#ifndef _SHIPPING
    true
#else
    false
#endif // _SHIPPING
};
// Remember to launch the vkconfig.exe under C:\VulkanSDK\1.2.x\Tools: 1. set layer change to be consistent, 2. select layer with VK_LAYER_KHRONOS_validation to be "Forced On"
const std::vector<const char*> VkRequestedValidationLayers {
    "VK_LAYER_KHRONOS_validation"
};
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
bool CheckRequiredValidationLayerSupport();
std::vector<const char*> GetRequiredExtensions();

// Physical device
struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    bool isComplete() {
        return graphicsFamily.has_value();
    }
};
QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);
bool IsDeviceSuitable(const vk::PhysicalDevice& device);

longmarch::VulkanContext::VulkanContext(GLFWwindow* windowHandle)
    :
    m_WindowHandle(windowHandle)
{
    ASSERT(windowHandle != nullptr, "Window handle is NULL!");
}

longmarch::VulkanContext::~VulkanContext()
{
    if constexpr (VkEnableValidationLayer) 
    {
        DestroyDebugUtilsMessengerEXT(*m_instance, m_debugCallback, nullptr);
    }
}

void longmarch::VulkanContext::Init()
{
    CreateInstance();
    SetUpValidationLayer();
    PickPhysicalDevice();
    CreateLogicalDevice();
}
 
void longmarch::VulkanContext::SwapBuffers()
{
}

void longmarch::VulkanContext::CreateInstance()
{
    ENGINE_INFO("Vulkan Info: ");
    if constexpr (VkEnableValidationLayer)
    {
        if (!CheckRequiredValidationLayerSupport())
        {
            ENGINE_EXCEPT(L"Vulkan validation layers requested, but not all are available!");
        }
    }

    auto appInfo = vk::ApplicationInfo(
        "LongMarch App",
        VK_MAKE_VERSION(0, 1, 0),
        "LongMarch Engine",
        VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_2
    );

    auto extensions = GetRequiredExtensions();
    auto createInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        0, 
        nullptr, // enabled layers
        static_cast<uint32_t>(extensions.size()), 
        extensions.data() // enabled extensions
    );

    if constexpr (VkEnableValidationLayer) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedValidationLayers.size());
        createInfo.ppEnabledLayerNames = VkRequestedValidationLayers.data();
    }

    VK_TRY_CATCH(
        m_instance = vk::createInstanceUnique(createInfo, nullptr),
        vk::SystemError,
        "Vulkan failed to create instance!"
    );
}

void longmarch::VulkanContext::SetUpValidationLayer()
{
    if constexpr (VkEnableValidationLayer)
    {
        auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            MyDebugReportCallback,
            nullptr
        );

        if (CreateDebugUtilsMessengerEXT(*m_instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &m_debugCallback) != VK_SUCCESS) 
        {
            ENGINE_EXCEPT(L"Vulkan failed to set up debug callback!");
        }
    }
}

void longmarch::VulkanContext::PickPhysicalDevice()
{
    auto devices = m_instance->enumeratePhysicalDevices();
    if (devices.size() == 0) 
    {
        ENGINE_EXCEPT(L"Vulkan failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : devices) 
    {
        if (IsDeviceSuitable(device)) 
        {
            m_physicalDevice = device;
            break;
        }
    }

    if (!m_physicalDevice)
    {
        ENGINE_EXCEPT(L"Vulkan failed to find a suitable GPU!");
    }
}

void longmarch::VulkanContext::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    float queuePriority = 1.0f;
    auto queueCreateInfo = vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(),
        indices.graphicsFamily.value(),
        1, // queueCount
        &queuePriority
    );

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    auto createInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(),
        1, 
        &queueCreateInfo
    );
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if constexpr (VkEnableValidationLayer) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VkRequestedValidationLayers.size());
        createInfo.ppEnabledLayerNames = VkRequestedValidationLayers.data();
    }

    VK_TRY_CATCH(
        m_device = m_physicalDevice.createDeviceUnique(createInfo),
        vk::SystemError,
        "Vulkan failed to create logical device!"
    );

    m_graphicsQueue = m_device->getQueue(indices.graphicsFamily.value(), 0);
}

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        //ENGINE_DEBUG("Vulkan Debug: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        //ENGINE_INFO("Vulkan Info: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ENGINE_WARN("Vulkan Warning: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_ERROR("Vulkan Error: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        ENGINE_CRITICAL("Vulkan Fatal: {0}", pCallbackData->pMessage);
        break;
    default:
        ENGINE_EXCEPT(L"Unkown vk::VkDebugUtilsMessageSeverityFlagBitsEXT!");
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

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = MyDebugReportCallback;
}

bool CheckRequiredValidationLayerSupport()
{
    // Get all available layers
    auto availableLayers = vk::enumerateInstanceLayerProperties();

    // Print all available layers
    for (const auto& layerProperties : availableLayers)
    {
        ENGINE_INFO("Vulkan Layer {0} is available.", layerProperties.layerName);
    }

    // Find requested layers
    for (const char* layerName : VkRequestedValidationLayers) 
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                ENGINE_INFO("Vulkan Layer resquested {0} is found!", layerName);
                break;
            }
        }
        if (!layerFound) 
        {
            ENGINE_WARN("Vulkan Layer resquested {0} is not found!", layerName);
            return false;
        }
    }
    return true;
}

std::vector<const char*> GetRequiredExtensions() 
{
    ENGINE_INFO("Vulkan Available Extensions:");
    for (const auto& extension : vk::enumerateInstanceExtensionProperties())
    {
        ENGINE_INFO(" \t{0}", extension.extensionName);
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if constexpr (VkEnableValidationLayer)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    ENGINE_INFO("Vulkan Requested Extensions:");
    for (const auto& extension : extensions)
    {
        ENGINE_INFO(" \t{0}", extension);
    }
    return extensions;
}

QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;
    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) 
        {
            indices.graphicsFamily = i;
        }
        if (indices.isComplete()) 
        {
            break;
        }
        ++i;
    }
    return indices;
}

bool IsDeviceSuitable(const vk::PhysicalDevice& device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);
    return indices.isComplete();
}