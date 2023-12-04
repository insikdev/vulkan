#pragma once

class Device;
class CommandPool;
class CommandBuffer;

class Resource {
protected:
    Resource(const Device*);
    virtual ~Resource();

public: // getter
    VkDeviceMemory GetDeviceMemory() { return m_deviceMemory; }

protected:
    CommandBuffer BeginSingleTimeCommand(void);
    void EndSingleTimeCommand(CommandBuffer);

protected:
    const Device* p_device;

protected:
    VkDeviceMemory m_deviceMemory { VK_NULL_HANDLE };

protected:
    static uint32_t s_count;
    static CommandPool* s_pool;
};
