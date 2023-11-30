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
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags) const;
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

public: // getter
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkDevice GetDevice() const { return m_device; }
    VkQueue GetQueue() const { return m_graphicsQueue; }
    VkQueue GetPresentQueue() const { return m_presentQueue; }
    QueueFamilyIndices GetQueueFamilyIndices() const { return m_queueFamilyIndices; }

private:
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice);
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
};
