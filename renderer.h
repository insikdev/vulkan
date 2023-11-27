#pragma once

class Device;
class SwapChain;
class Pipeline;
class Scene;
class DescriptorHelper;

class Renderer {
public:
    Renderer(const Device*, const SwapChain*, const Pipeline*);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

public:
    void DrawFrame(const Scene*);
    void UpdateSwapChain(const SwapChain*);

private:
    void createDescriptorSets();
    void createDescriptorPool();
    void createUniformBuffers();
    void CreateFrameBuffers();
    void CreateSyncObjects();
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer, uint32_t imageIndex, const Scene*);

private:
    const Device* p_device;
    const SwapChain* p_swapChain;
    const Pipeline* p_pipeline;
    const DescriptorHelper* p_descriptor;

private:
    enum { MAX_FRAMES_IN_FLIGHT = 2 };

private:
    VkClearValue m_clearColor { 0.0f, 0.0f, 0.0f, 1.0f };
    std::vector<VkFramebuffer> m_frameBuffers;
    std::vector<VkCommandBuffer> m_commandBuffers { MAX_FRAMES_IN_FLIGHT };
    std::vector<VkSemaphore> imageAvailableSemaphores { MAX_FRAMES_IN_FLIGHT };
    std::vector<VkSemaphore> renderFinishedSemaphores { MAX_FRAMES_IN_FLIGHT };
    std::vector<VkFence> inFlightFences { MAX_FRAMES_IN_FLIGHT };
    uint32_t m_currentFrame {};

private:
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

private:
    std::vector<VkDescriptorSet> descriptorSets;
};
