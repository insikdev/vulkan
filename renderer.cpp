#include "renderer.h"
#include <iostream>
#include <stdexcept>

Renderer::Renderer()
{
    InitWindow();
    InitVulkan();
}

Renderer::~Renderer()
{
    vkDestroyInstance(m_instance, nullptr);
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

void Renderer::InitVulkan()
{
    CreateInstance();
}

void Renderer::CreateInstance()
{

    VkApplicationInfo appInfo {};
    {
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "vulkan tutorial";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    {
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::cout << "required instance extensions : " << glfwExtensionCount << '\n';
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            std::cout << '\t' << glfwExtensions[i] << '\n';
        }
        std::cout << '\n';
    }

    uint32_t availableExtensionCount = 0;
    std::vector<VkExtensionProperties> extensions;
    {
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
        extensions.resize(availableExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, extensions.data());

        std::cout << "available extensions : " << availableExtensionCount << '\n';
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << '\n';
    }

    VkInstanceCreateInfo createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    };
}
