#include "engine-precompiled-header.h"
#include "VulkanContext.h"

#ifndef _SHIPPING
constexpr bool VkEnableValidationLayer{ false };
#else
constexpr bool VkEnableValidationLayer{ false };
#endif // _SHIPPING
const std::vector<const char*> VkValidationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
bool CheckValidationLayerSupport();
std::vector<const char*> GetRequiredExtensions();

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
    SetUpValidationLayerDebugMessenger();
}
 
void longmarch::VulkanContext::SwapBuffers()
{
}

void longmarch::VulkanContext::CreateInstance()
{
    if constexpr (VkEnableValidationLayer)
    {
        if (!CheckValidationLayerSupport())
        {
            ENGINE_WARN("Vulkan validation layers requested, but not available!");
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
        createInfo.enabledLayerCount = static_cast<uint32_t>(VkValidationLayers.size());
        createInfo.ppEnabledLayerNames = VkValidationLayers.data();
    }

    VK_TRY_CATCH(
        m_instance = vk::createInstanceUnique(createInfo, nullptr),
        vk::SystemError,
        "Vulkan failed to create instance!"
    );

    ENGINE_INFO("Vulkan Info: ");
    ENGINE_INFO("Vulkan Available Extensions:");
    for (const auto& extension : vk::enumerateInstanceExtensionProperties())
    {
        ENGINE_INFO(" \t{0}", extension.extensionName);
    }
}

void longmarch::VulkanContext::SetUpValidationLayerDebugMessenger()
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

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        ENGINE_DEBUG("Vulkan Warning: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        ENGINE_INFO("Vulkan Warning: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        ENGINE_WARN("Vulkan Warning: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ENGINE_ERROR("Vulkan Error: {0}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        ENGINE_CRITICAL("Vulkan Error: {0}", pCallbackData->pMessage);
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

bool CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VkValidationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return false;
}

std::vector<const char*> GetRequiredExtensions() 
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if constexpr (VkEnableValidationLayer)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}
