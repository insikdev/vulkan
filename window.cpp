#include "pch.h"
#include "window.h"

Window::Window(const char* title)
{
    CreateGLFWWindow(title);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void Window::CreateGLFWWindow(const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    assert(m_window);
}