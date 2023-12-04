#include "pch.h"
#include "vk_instance.h"
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
    VkApplicationInfo appInfo { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    {
        appInfo.pApplicationName = "vulkan_tutorial";
        appInfo.pEngineName = "tutorial";
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    Extension::populateDebugMessengerCreateInfo(debugCreateInfo);

    VkInstanceCreateInfo createInfo { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    {
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.pNext = &debugCreateInfo;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    CHECK_VK(result);
}
