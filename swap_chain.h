#pragma once

class Window;
class Instance;
class Device;

class SwapChain {
public:
    SwapChain(const Window*, const Instance*, const Device*);
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

public: // getter
    inline VkExtent2D GetExtent2D() const { return m_extent; }
    inline VkFormat GetFormat() const { return m_format.format; }
    inline std::vector<VkImageView> GetImageViews() const { return m_imageViews; }
    inline VkSwapchainKHR GetSwapChain() const { return m_swapChain; }

private:
    void CreateSurface();
    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats();
    void SelectAppropriateSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    void SelectPresentMode();
    void CreateSwapChain();
    void CreateImageViews();

private:
    const Window* p_window;
    const Instance* p_instance;
    const Device* p_device;

private:
    VkSurfaceKHR m_surface;
    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent;
    VkPresentModeKHR m_presentMode { VK_PRESENT_MODE_FIFO_KHR };
    VkSwapchainKHR m_swapChain;
    std::vector<VkImageView> m_imageViews;
};
