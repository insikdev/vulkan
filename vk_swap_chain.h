#pragma once

class Window;
class Surface;
class Device;

class SwapChain {
public:
    SwapChain(const Window*, const Surface*, const Device*);
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

public: // getter
    VkExtent2D GetExtent2D() const { return m_extent; }
    VkFormat GetFormat() const { return m_surfaceFormat.format; }
    std::vector<VkImageView> GetImageViews() const { return m_imageViews; }
    VkSwapchainKHR GetSwapChain() const { return m_swapChain; }
    VkFramebuffer GetFrameBuffer(uint32_t i) const { return m_frameBuffers[i]; }
    uint32_t GetImageCount() { return m_imageCount; }

public:
    void CreateFrameBuffer(VkRenderPass);

private:
    void SelectSurfaceFormat();
    void SelectPresentMode();
    void SelectCapabilities();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

public:
    VkFormat findDepthFormat() const
    {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

private:
    const Window* p_window;
    const Surface* p_surface;
    const Device* p_device;

private:
    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode { VK_PRESENT_MODE_FIFO_KHR };
    VkExtent2D m_extent;
    uint32_t m_imageCount;
    VkSurfaceTransformFlagBitsKHR m_currentTransform;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_frameBuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};
