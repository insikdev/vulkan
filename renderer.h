#pragma once

class Device;
class SwapChain;
class Pipeline;
class Scene;
class CommandPool;
class CommandBuffer;
class DescriptorPool;
class Image;

struct PerFrame {
    CommandPool* p_commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
};

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
    void InitPerFrame();
    void RecordCommandBuffer(VkCommandBuffer, uint32_t imageIndex);

private: // temp
    void CreateTextureImage();
    void CreateSampler();
    Image* p_image;
    VkSampler textureSampler;
    VkImageView imageView;

private:
    Device* p_device;
    SwapChain* p_swapChain;
    const Pipeline* p_pipeline;
    DescriptorPool* p_descriptorPool;
    Scene* p_scene;

private:
    enum { MAX_FRAMES_IN_FLIGHT = 3 };
    PerFrame m_frames[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame = 0;
};
