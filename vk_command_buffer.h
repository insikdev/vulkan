#pragma once

class Device;

class CommandBuffer {
    friend class CommandPool;

public:
    CommandBuffer() = delete;
    ~CommandBuffer() = default;

private:
    CommandBuffer(const Device*, VkCommandBuffer);

public:
    void Begin();
    void End();

public: // getter
    VkCommandBuffer GetHandle() const { return m_commandBuffer; }
    VkCommandBuffer* GetPtr() { return &m_commandBuffer; }

private:
    const Device* p_device;

private:
    VkCommandBuffer m_commandBuffer;
};
