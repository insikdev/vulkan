#pragma once

class Instance;

class Device {
public:
    Device(const Instance*, const std::vector<const char*>& extensions);
    ~Device();
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

public:
    void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&) const;

public: // getter
    inline VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    inline VkDevice GetDevice() const { return m_device; }
    inline VkQueue GetQueue() const { return m_queue; }
    inline VkCommandPool GetCommandPool() const { return m_commandPool; }
    inline VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

private:
    std::vector<VkPhysicalDevice> EnumeratePhysicalDevices();
    void SelectAppropriatePhysicalDevice(const std::vector<VkPhysicalDevice>&);
    std::vector<VkQueueFamilyProperties> GetQueueFamilies();
    void SelectAppropriateQueueFamily(const std::vector<VkQueueFamilyProperties>&);
    void CreateLogicalDeviceAndQueue(const std::vector<const char*>&);
    void CreateCommandPool();
    void CreateCommandBuffer();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags) const;

private:
    const Instance* p_instance;

private:
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    uint32_t m_graphicsQueueIndex { UINT32_MAX };
    VkQueue m_queue;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
};
