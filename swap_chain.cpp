#include "pch.h"
#include "swap_chain.h"
#include "window.h"
#include "instance.h"
#include "device.h"

SwapChain::SwapChain(const Window* pWindow, const Instance* pInstance, const Device* pDevice)
    : p_window { pWindow }
    , p_instance { pInstance }
    , p_device { pDevice }
{
    CreateSurface();
    const auto& formats = GetSurfaceFormats();
    SelectAppropriateSurfaceFormat(formats);
    SelectPresentMode();
    CreateSwapChain();
    CreateImageViews();
}

SwapChain::~SwapChain()
{
    for (auto imageView : m_imageViews) {
        vkDestroyImageView(p_device->GetDevice(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(p_device->GetDevice(), m_swapChain, nullptr);
    vkDestroySurfaceKHR(p_instance->GetInstance(), m_surface, nullptr);
}

void SwapChain::CreateSurface()
{
    VkResult result = glfwCreateWindowSurface(p_instance->GetInstance(), p_window->GetWindow(), nullptr, &m_surface);
    CHECK_VK(result);
}

std::vector<VkSurfaceFormatKHR> SwapChain::GetSurfaceFormats()
{
    VkResult result;

    uint32_t count = 0;
    std::vector<VkSurfaceFormatKHR> formats;

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(p_device->GetPhysicalDevice(), m_surface, &count, nullptr);
    CHECK_VK(result);
    assert(count != 0);

    formats.resize(count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(p_device->GetPhysicalDevice(), m_surface, &count, formats.data());
    CHECK_VK(result);

    return formats;
}

void SwapChain::SelectAppropriateSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats)
{
    m_format = surfaceFormats[0];
}

void SwapChain::SelectPresentMode()
{
    // m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // return;

    VkResult result;

    uint32_t count;
    std::vector<VkPresentModeKHR> presentModes;

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(p_device->GetPhysicalDevice(), m_surface, &count, nullptr);
    CHECK_VK(result);
    assert(count != 0);

    presentModes.resize(count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(p_device->GetPhysicalDevice(), m_surface, &count, presentModes.data());
    CHECK_VK(result);

    for (const auto& mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            m_presentMode = mode;
            break;
        }
    }
}

void SwapChain::CreateSwapChain()
{
    VkResult result;

    VkSurfaceCapabilitiesKHR capabilities;
    {
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device->GetPhysicalDevice(), m_surface, &capabilities);
        CHECK_VK(result);
        m_extent = capabilities.currentExtent;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = capabilities.minImageCount + 1;
    createInfo.imageFormat = m_format.format;
    createInfo.imageColorSpace = m_format.colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = m_presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(p_device->GetDevice(), &createInfo, nullptr, &m_swapChain);
    CHECK_VK(result);
}

void SwapChain::CreateImageViews()
{
    VkResult result;

    uint32_t imageCount = 0;
    std::vector<VkImage> images;

    result = vkGetSwapchainImagesKHR(p_device->GetDevice(), m_swapChain, &imageCount, nullptr);
    CHECK_VK(result);
    assert(imageCount != 0);

    images.resize(imageCount);
    vkGetSwapchainImagesKHR(p_device->GetDevice(), m_swapChain, &imageCount, images.data());
    CHECK_VK(result);

    m_imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_format.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(p_device->GetDevice(), &createInfo, nullptr, &m_imageViews[i]);
        CHECK_VK(result);
    }
}
