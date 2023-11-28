#pragma once

class Window;
class Instance;
class Surface;
class Device;
class SwapChain;
class Pipeline;
class Renderer;
class Scene;

class App {
public:
    App();
    ~App();

public:
    void Run();
    void SetResizedTrue() { m_resized = true; }

private:
    void SetupDebugMessenger();
    void CalculateFrameRate();
    void HandleResize();
    void InitGui();

private:
    std::vector<const char*> m_requiredLayers { "VK_LAYER_KHRONOS_validation" };
    // std::vector<const char*> m_requiredLayers { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_api_dump" };
    std::vector<const char*> m_requiredLayerExtensions {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
    std::vector<const char*> m_requiredDeviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

private:
    VkDebugUtilsMessengerEXT m_debugMessenger;

private:
    Window* p_window;
    Instance* p_instance;
    Surface* p_surface;
    Device* p_device;
    SwapChain* p_swapChain;
    Pipeline* p_pipeline;
    Renderer* p_renderer;
    Scene* p_scene;

private:
    double lastTime {};
    double currentTime {};
    int numFrames {};
    float frameTime {};
    bool m_resized { false };
    VkDescriptorPool m_pool;
};
