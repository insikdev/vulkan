#pragma once

class Device;
class SwapChain;
class Pipeline;
class Scene;
class CommandPool;
class CommandBuffer;
class DescriptorPool;

class Renderer {
public:
    Renderer(Device*, SwapChain*, const Pipeline*);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

public:
    void SetScene(Scene*);
    void Update(float dt);
    void Render();
    void UpdateSwapChain(SwapChain*);

private:
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer, uint32_t imageIndex);

private:
    Device* p_device;
    SwapChain* p_swapChain;
    const Pipeline* p_pipeline;
    CommandPool* p_commandPool;
    DescriptorPool* p_descriptorPool;
    Scene* p_scene;

private:
    // enum { MAX_FRAMES_IN_FLIGHT = 1 };

private:
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkCommandBuffer m_commandBuffer;
};
