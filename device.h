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
    // void CreateCommandBuffer(VkCommandBuffer&) const;
    void CreateCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers) const;
    void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&) const;
    void CopyBuffer(VkBuffer, VkBuffer, VkDeviceSize) const;

public:
    static std::vector<VkQueueFamilyProperties> GetQueueFamilies(VkPhysicalDevice);
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);

public: // getter
    inline VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    inline VkDevice GetDevice() const { return m_device; }
    inline VkQueue GetQueue() const { return m_graphicsQueue; }
    inline VkQueue GetPresentQueue() const { return m_presentQueue; }
    inline VkCommandPool GetCommandPool() const { return m_commandPool; }

private:
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();

private:
    std::vector<VkPhysicalDevice> EnumeratePhysicalDevices();
    bool IsDeviceSuitable(VkPhysicalDevice);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags) const;

private:
    const Instance* p_instance;
    const Surface* p_surface;

private:
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    QueueFamilyIndices m_queueFamilyIndices;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkCommandPool m_commandPool;
    std::vector<const char*> m_requiredExtensions;
};
