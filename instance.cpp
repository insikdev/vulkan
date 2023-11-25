#include "pch.h"
#include "instance.h"
#include "extension.h"

Instance::Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    CreateInstance(layers, extensions);
}

Instance::~Instance()
{
    vkDestroyInstance(m_instance, nullptr);
}

void Instance::CreateInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "tutorial";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "engine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    Extension::populateDebugMessengerCreateInfo(debugCreateInfo);

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.pNext = &debugCreateInfo;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    CHECK_VK(result);
}
