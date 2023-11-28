#pragma once

class Device;
class SwapChain;
class Pipeline;
class Scene;

class Renderer {
public:
    Renderer(Device*, SwapChain*, const Pipeline*);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

public:
    void Update(const Scene*, float dt);
    void Render(const Scene*);
    void UpdateSwapChain(SwapChain*);
    VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

private:
    VkDescriptorSet createDescriptorSet(std::vector<VkDescriptorSetLayout>);
    void createDescriptorPool();
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer, uint32_t imageIndex, const Scene*);

private:
    Device* p_device;
    SwapChain* p_swapChain;
    const Pipeline* p_pipeline;

private:
    enum { MAX_FRAMES_IN_FLIGHT = 1 };

private:
    VkClearValue m_clearColor { 0.0f, 0.0f, 0.0f, 1.0f };
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    // std::vector<VkFramebuffer> m_frameBuffers;
    // std::vector<VkSemaphore> imageAvailableSemaphores { MAX_FRAMES_IN_FLIGHT };
    // std::vector<VkSemaphore> renderFinishedSemaphores { MAX_FRAMES_IN_FLIGHT };
    // std::vector<VkFence> inFlightFences { MAX_FRAMES_IN_FLIGHT };
    // uint32_t m_currentFrame {};

private:
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
};
