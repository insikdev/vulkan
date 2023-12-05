#pragma once

#include "vk_resource.h"
class Device;

class Buffer : public Resource {
public:
    Buffer() = delete;
    Buffer(const Device*, VkBufferCreateInfo, VkMemoryPropertyFlags);
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

public:
    void MapMemory(void);
    void UnmapMemory(void);
    void InvalidateMappedMemory(void);
    void FlushMappedMemory(void);
    void Copy(VkBuffer);

public: // getter
    VkBuffer GetBuffer() { return m_buffer; }
    void* GetMappedPtr() { return p_host; }

private:
    void CreateBuffer(VkBufferCreateInfo, VkMemoryPropertyFlags);

private:
    VkDeviceSize m_size;
    VkBuffer m_buffer;
    void* p_host;
};
