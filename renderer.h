#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

public:
    void Run();

private:
    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    std::vector<const char*> GetRequiredExtensions();
    bool CheckValidationLayerSupport();
    void SetupDebugMessenger();

private:
    VkDebugUtilsMessengerEXT debugMessenger;
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

private:
    GLFWwindow* m_window;
    VkInstance m_instance;
    std::vector<const char*> m_layers { "VK_LAYER_KHRONOS_validation" };

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
