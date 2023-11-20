#include "renderer.h"
#include <iostream>
#include <stdexcept>

Renderer::Renderer()
{
    InitWindow();
}

Renderer::~Renderer()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Renderer::Run()
{
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void Renderer::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(800, 600, "vulkan tutorial", nullptr, nullptr);
}
