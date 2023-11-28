#include "pch.h"
#include "device.h"
#include "instance.h"
#include "surface.h"
#include "swap_chain.h"
#include "query.h"

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
    vkDestroyCommandPool(m_device, m_commandPool, 0);
    vkDestroyDevice(m_device, nullptr);
}

void Device::AllocateCommandBuffers(uint32_t count)
{
    VkCommandBufferAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;
    }

    m_commandBuffers.resize(count);

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data());
    CHECK_VK(result);
}

void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& deviceMemory) const
{
    VkBufferCreateInfo bufferInfo {};
    {
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    CHECK_VK(result);

    VkMemoryRequirements memRequirements {};
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
    }

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &deviceMemory);
    CHECK_VK(result);

    vkBindBufferMemory(m_device, buffer, deviceMemory, 0);
}

void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    VkCommandBufferAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;
    }

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion {};
    {
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
    }

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
    }

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);
    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Device::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const
{
    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image, imageMemory, 0);
}

VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const
{
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Device::SelectPhysicalDevice()
{
    const auto& physicalDevices = Query::GetPhysicalDevices(p_instance->GetInstance());

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
    m_queueFamilyIndices = FindQueueFamily(m_physicalDevice, p_surface->GetSurface());

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

    VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    CHECK_VK(result);

    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily, 0, &m_presentQueue);
}

void Device::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily;
    }

    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    CHECK_VK(result);
}

bool Device::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkPhysicalDeviceMemoryProperties memoryProperties {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    VkPhysicalDeviceFeatures features {};
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    QueueFamilyIndices indices = FindQueueFamily(physicalDevice, p_surface->GetSurface());

    if (!indices.isComplete()) {
        return false;
    }

    if (!CheckDeviceExtensionSupport(physicalDevice)) {
        return false;
    }

    if (Query::GetSurfaceFormats(physicalDevice, p_surface->GetSurface()).empty() || Query::GetPresentModes(physicalDevice, p_surface->GetSurface()).empty()) {
        return false;
    }

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
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

QueueFamilyIndices Device::FindQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    const auto& queueFamilies = Query::GetQueueFamilyProperties(physicalDevice);

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