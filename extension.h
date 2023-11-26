#pragma once

struct ExtensionInfo {
    VkLayerProperties layerProperty;
    std::vector<VkExtensionProperties> extensionProperties;
};

class Extension {
public:
    static std::vector<ExtensionInfo> GetAvailableInstanceExtensions(void);
    static std::vector<ExtensionInfo> GetAvailableDeviceExtensions(VkPhysicalDevice);
    static void PrintExtensionInfos(const std::vector<ExtensionInfo>&);

    // debug messenger
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
    static std::vector<VkLayerProperties> GetInstanceLayerProperties();
};
