#include "pch.h"
#include "device.h"
#include "instance.h"

Device::Device(const Instance* pInstance, const std::vector<const char*>& extensions)
    : p_instance { pInstance }
{
    const auto& physicalDevices = EnumeratePhysicalDevices();
    SelectAppropriatePhysicalDevice(physicalDevices);
    const auto& queueFamilies = GetQueueFamilies();
    SelectAppropriateQueueFamily(queueFamilies);
    CreateLogicalDeviceAndQueue(extensions);
    CreateCommandPool();
    CreateCommandBuffer();
}

Device::~Device()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
}

void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& deviceMemory) const
{
    VkResult result;

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    CHECK_VK(result);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &deviceMemory);
    CHECK_VK(result);

    vkBindBufferMemory(m_device, buffer, deviceMemory, 0);
}

std::vector<VkPhysicalDevice> Device::EnumeratePhysicalDevices()
{
    VkResult result;

    uint32_t count = 0;
    std::vector<VkPhysicalDevice> devices;

    result = vkEnumeratePhysicalDevices(p_instance->GetInstance(), &count, nullptr);
    CHECK_VK(result);
    assert(count != 0);

    devices.resize(count);
    result = vkEnumeratePhysicalDevices(p_instance->GetInstance(), &count, devices.data());
    CHECK_VK(result);

    return devices;
}

void Device::SelectAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>& physicalDevices)
{
    if (physicalDevices.size() == 1) {
        m_physicalDevice = physicalDevices[0];
        return;
    }

    for (const auto& physicalDevice : physicalDevices) {
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkPhysicalDeviceMemoryProperties memoryProperties {};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        VkPhysicalDeviceFeatures features {};
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            m_physicalDevice = physicalDevice;
            break;
        }
    }

    assert(m_physicalDevice);
}

std::vector<VkQueueFamilyProperties> Device::GetQueueFamilies()
{
    uint32_t count = 0;
    std::vector<VkQueueFamilyProperties> queueFamilies;

    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
    assert(count != 0);

    queueFamilies.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, queueFamilies.data());

    return queueFamilies;
}

void Device::SelectAppropriateQueueFamily(const std::vector<VkQueueFamilyProperties>& queueFamilies)
{
    if (queueFamilies.size() == 1) {
        m_graphicsQueueIndex = 0;
        return;
    }

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsQueueIndex = i;
            break;
        }
    }

    assert(m_graphicsQueueIndex != UINT32_MAX);
}

void Device::CreateLogicalDeviceAndQueue(const std::vector<const char*>& extensions)
{
    VkResult result;

    float queuePriorities = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = m_graphicsQueueIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    CHECK_VK(result);

    vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_queue);
}

void Device::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueIndex;

    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    CHECK_VK(result);
}

void Device::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);
    CHECK_VK(result);
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties {};
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}