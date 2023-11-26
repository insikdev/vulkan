#include "pch.h"
#include "extension.h"

std::vector<VkLayerProperties> Extension::GetInstanceLayerProperties()
{
    VkResult result;
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties;

    result = vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    CHECK_VK(result);

    if (propertyCount != 0) {
        layerProperties.resize(propertyCount);
        result = vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());
        CHECK_VK(result);
    }

    return layerProperties;
}

std::vector<ExtensionInfo> Extension::GetAvailableInstanceExtensions()
{
    VkResult result;
    std::vector<ExtensionInfo> extensions;
    auto instanceLayerProperties = Extension::GetInstanceLayerProperties();

    for (const auto& layerProperty : instanceLayerProperties) {
        ExtensionInfo info {};
        info.layerProperty = layerProperty;

        uint32_t extensionCount = 0;
        std::vector<VkExtensionProperties> extensionProperties;

        result = vkEnumerateInstanceExtensionProperties(layerProperty.layerName, &extensionCount, nullptr);
        CHECK_VK(result);

        if (extensionCount != 0) {
            extensionProperties.resize(extensionCount);
            result = vkEnumerateInstanceExtensionProperties(layerProperty.layerName, &extensionCount, extensionProperties.data());
            CHECK_VK(result);

            info.extensionProperties = extensionProperties;
        }

        extensions.push_back(info);
    }

    return extensions;
}

std::vector<ExtensionInfo> Extension::GetAvailableDeviceExtensions(VkPhysicalDevice device)
{
    VkResult result;
    std::vector<ExtensionInfo> extensions;
    auto instanceLayerProperties = Extension::GetInstanceLayerProperties();

    for (const auto& layerProperty : instanceLayerProperties) {
        ExtensionInfo info {};
        info.layerProperty = layerProperty;

        uint32_t deviceCount = 0;
        std::vector<VkExtensionProperties> extensionProperties;

        result = vkEnumerateDeviceExtensionProperties(device, layerProperty.layerName, &deviceCount, nullptr);
        CHECK_VK(result);

        if (deviceCount != 0) {
            extensionProperties.resize(deviceCount);
            result = vkEnumerateDeviceExtensionProperties(device, layerProperty.layerName, &deviceCount, extensionProperties.data());
            CHECK_VK(result);

            info.extensionProperties = extensionProperties;
        }

        extensions.push_back(info);
    }

    return extensions;
}

void Extension::PrintExtensionInfos(const std::vector<ExtensionInfo>& extensionInfos)
{
    std::cout << "Supported Extension Info\n";

    for (const auto& info : extensionInfos) {
        std::cout << info.layerProperty.layerName << '\n';

        if (info.extensionProperties.empty()) {
            continue;
        }

        for (const auto& extension : info.extensionProperties) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    std::cout << '\n';
}

VKAPI_ATTR VkBool32 VKAPI_CALL Extension::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult Extension::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Extension::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void Extension::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}