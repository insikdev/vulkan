#pragma once

class Device;
class CommandPool;

class Buffer {
public:
    Buffer() = delete;
    Buffer(const Device*, VkBufferCreateInfo&, VkMemoryPropertyFlags);
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

public:
    void Copy(VkBuffer);
    void MapMemory(void);
    void UnmapMemory(void);
    void InvalidateMappedMemory(void);
    void FlushMappedMemory(void);

public: // getter
    VkBuffer GetBuffer() { return m_buffer; }
    VkDeviceMemory GetDeviceMemory() { return m_deviceMemory; }
    void* GetMappedPtr() { return p_host; }

private:
    void CreateBuffer(VkBufferCreateInfo&, VkMemoryPropertyFlags);

private:
    const Device* p_device;

private:
    VkDeviceSize m_size;
    VkBuffer m_buffer;
    VkDeviceMemory m_deviceMemory;
    void* p_host;

private:
    static uint32_t s_count;
    static CommandPool* s_pool;
};
