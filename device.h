#pragma once

class Instance;
class Surface;

struct QueueFamilyIndices {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    bool isComplete() { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX; }
};

class Device {
public:
    Device(const Instance*, const Surface*, const std::vector<const char*>& extensions);
    ~Device();
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

public:
    void AllocateCommandBuffers(uint32_t count);
    VkCommandBuffer GetCommandBuffer(uint32_t i) { return m_commandBuffers[i]; }
    void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&) const;
    void CopyBuffer(VkBuffer, VkBuffer, VkDeviceSize) const;
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

public: // getter
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkDevice GetDevice() const { return m_device; }
    VkQueue GetQueue() const { return m_graphicsQueue; }
    VkQueue GetPresentQueue() const { return m_presentQueue; }
    QueueFamilyIndices GetQueueFamilyIndices() const { return m_queueFamilyIndices; }
    VkCommandPool GetCommandPool() { return m_commandPool; }

private:
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();

private:
    bool IsDeviceSuitable(VkPhysicalDevice);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags) const;
    QueueFamilyIndices FindQueueFamily(VkPhysicalDevice, VkSurfaceKHR);

private:
    const Instance* p_instance;
    const Surface* p_surface;

private:
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    QueueFamilyIndices m_queueFamilyIndices;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    std::vector<const char*> m_requiredExtensions;
    //
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
};
