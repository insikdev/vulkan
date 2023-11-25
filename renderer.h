#pragma once

class Device;
class SwapChain;
class Pipeline;
class Scene;

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

private:
    void CreateFrameBuffers();
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer, uint32_t imageIndex, const Scene*);

private:
    const Device* p_device;
    const SwapChain* p_swapChain;
    const Pipeline* p_pipeline;

private:
    VkClearValue m_clearColor { 0.5f, 0.5f, 0.5f, 1.0f };
    std::vector<VkFramebuffer> m_frameBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
};
