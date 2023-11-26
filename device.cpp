#include "pch.h"
#include "device.h"
#include "instance.h"
#include "surface.h"
#include "swap_chain.h"
#include "set"

Device::Device(const Instance* pInstance, const Surface* pSurface, const std::vector<const char*>& extensions)
    : p_instance { pInstance }
    , p_surface { pSurface }
    , m_requiredExtensions { extensions }
{
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateCommandPool();
}

Device::~Device()
{
    /*
     * VkPhysicalDevice will be implicitly destroyed when the VkInstance is destroyed.
     */
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
}

void Device::CreateCommandBuffer(VkCommandBuffer& commandBuffer) const
{
    VkCommandBufferAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
    }

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
    CHECK_VK(result);
}

void Device::CreateCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers) const
{
    VkCommandBufferAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    }

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers.data());
    CHECK_VK(result);
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

QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    const auto& queueFamilies = Device::GetQueueFamilies(physicalDevice);

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

void Device::SelectPhysicalDevice()
{
    const auto& physicalDevices = EnumeratePhysicalDevices();

    for (const auto& physicalDevice : physicalDevices) {
        if (IsDeviceSuitable(physicalDevice)) {
            m_physicalDevice = physicalDevice;
            break;
        }
    }

    assert(m_physicalDevice);
}

void Device::CreateLogicalDevice()
{
    VkResult result;
    m_queueFamilyIndices = Device::FindQueueFamilies(m_physicalDevice, p_surface->GetSurface());

    std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilyIndices.graphicsFamily, m_queueFamilyIndices.presentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo deviceCreateInfo {};
    {
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = m_requiredExtensions.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    }

    result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    CHECK_VK(result);

    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily, 0, &m_presentQueue);
}

void Device::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily;

    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    CHECK_VK(result);
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

bool Device::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkPhysicalDeviceMemoryProperties memoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    VkPhysicalDeviceFeatures features {};
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    QueueFamilyIndices indices = Device::FindQueueFamilies(physicalDevice, p_surface->GetSurface());

    if (!indices.isComplete()) {
        return false;
    }

    if (!CheckDeviceExtensionSupport(physicalDevice)) {
        return false;
    }

    if (SwapChain::GetSurfaceFormats(physicalDevice, p_surface->GetSurface()).empty() || SwapChain::GetPresentModes(physicalDevice, p_surface->GetSurface()).empty()) {
        return false;
    }

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

std::vector<VkQueueFamilyProperties> Device::GetQueueFamilies(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0;
    std::vector<VkQueueFamilyProperties> queueFamilies;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    assert(count != 0);

    queueFamilies.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilies.data());

    return queueFamilies;
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_requiredExtensions.begin(), m_requiredExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
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