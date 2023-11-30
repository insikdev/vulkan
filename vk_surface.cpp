#include "pch.h"
#include "vk_surface.h"
#include "vk_window.h"
#include "vk_instance.h"

Surface::Surface(const Window* pWindow, const Instance* pInstance)
    : p_window { pWindow }
    , p_instance { pInstance }
{
    CreateSurface();
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(p_instance->GetInstance(), m_surface, nullptr);
}

void Surface::CreateSurface()
{
    VkResult result = glfwCreateWindowSurface(p_instance->GetInstance(), p_window->GetWindow(), nullptr, &m_surface);
    CHECK_VK(result);
}
