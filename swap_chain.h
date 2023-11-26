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
    inline VkExtent2D GetExtent2D() const { return m_extent; }
    inline VkFormat GetFormat() const { return m_surfaceFormat.format; }
    inline std::vector<VkImageView> GetImageViews() const { return m_imageViews; }
    inline VkSwapchainKHR GetSwapChain() const { return m_swapChain; }

public:
    static std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice, VkSurfaceKHR);
    static std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice, VkSurfaceKHR);

private:
    void SelectSurfaceFormat();
    void SelectPresentMode();
    void SelectCapabilities();
    void CreateSwapChain();
    void CreateImageViews();

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
};
