#pragma once

class Device;
class CommandBuffer;

class CommandPool {
public:
    CommandPool(const Device*);
    ~CommandPool();
    CommandPool(const CommandPool&) = delete;
    CommandPool(CommandPool&&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;
    CommandPool& operator=(CommandPool&&) = delete;

public:
    CommandBuffer AllocateCommandBuffer();
    void FreeCommandBuffer(CommandBuffer&);

public: // getter
    VkCommandPool GetPool() { return m_pool; }

private:
    void CreatePool();

private:
    const Device* p_device;

private:
    VkCommandPool m_pool;
};
