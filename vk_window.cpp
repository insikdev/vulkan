#include "pch.h"
#include "vk_window.h"
#include "app.h"

Window::Window(const char* title, App* pApp)
    : p_app { pApp }
{
    CreateGLFWWindow(title);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

    app->SetResizedTrue();
}

void Window::CreateGLFWWindow(const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    assert(m_window);

    glfwSetWindowUserPointer(m_window, p_app);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}