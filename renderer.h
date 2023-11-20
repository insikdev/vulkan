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

private:
    GLFWwindow* m_window;
    VkInstance m_instance;
};
