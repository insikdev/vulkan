#include "pch.h"
#include "swap_chain.h"
#include "window.h"
#include "surface.h"
#include "device.h"
#include "query.h"

SwapChain::SwapChain(const Window* pWindow, const Surface* pSurface, const Device* pDevice)
    : p_window { pWindow }
    , p_surface { pSurface }
    , p_device { pDevice }
{
    SelectSurfaceFormat();
    SelectPresentMode();
    SelectCapabilities();
    CreateSwapChain();
    CreateImageViews();
    CreateDepthResources();
}

SwapChain::~SwapChain()
{
    vkDestroyImageView(p_device->GetDevice(), depthImageView, nullptr);
    vkDestroyImage(p_device->GetDevice(), depthImage, nullptr);
    vkFreeMemory(p_device->GetDevice(), depthImageMemory, nullptr);
    /*
     * VkImage will be implicitly destroyed when the VkSwapchainKHR is destroyed.
     */
    for (auto framebuffer : m_frameBuffers) {
        vkDestroyFramebuffer(p_device->GetDevice(), framebuffer, nullptr);
    }

    for (auto imageView : m_imageViews) {
        vkDestroyImageView(p_device->GetDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(p_device->GetDevice(), m_swapChain, nullptr);
}

void SwapChain::CreateFrameBuffer(VkRenderPass renderPass)
{
    m_frameBuffers.resize(m_imageViews.size());

    for (size_t i = 0; i < m_frameBuffers.size(); i++) {
        std::array<VkImageView, 2> attachments = { m_imageViews[i], depthImageView };

        VkFramebufferCreateInfo framebufferInfo {};
        {
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_extent.width;
            framebufferInfo.height = m_extent.height;
            framebufferInfo.layers = 1;
        }

        VkResult result = vkCreateFramebuffer(p_device->GetDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]);
        CHECK_VK(result);
    }
}

void SwapChain::SelectSurfaceFormat()
{
    const auto& availableFormats = Query::GetSurfaceFormats(p_device->GetPhysicalDevice(), p_surface->GetSurface());

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            m_surfaceFormat = availableFormat;
            return;
        }
    }

    m_surfaceFormat = availableFormats[0];
}

void SwapChain::SelectPresentMode()
{
    // m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // return;

    const auto& presentModes = Query::GetPresentModes(p_device->GetPhysicalDevice(), p_surface->GetSurface());

    for (const auto& mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            m_presentMode = mode;
            return;
        }
    }

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::SelectCapabilities()
{
    VkSurfaceCapabilitiesKHR capabilities {};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device->GetPhysicalDevice(), p_surface->GetSurface(), &capabilities);
    CHECK_VK(result);

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        m_extent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(p_window->GetWindow(), &width, &height);

        VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        m_extent = actualExtent;
    }

    m_imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && m_imageCount > capabilities.maxImageCount) {
        m_imageCount = capabilities.maxImageCount;
    }

    m_currentTransform = capabilities.currentTransform;
}

void SwapChain::CreateSwapChain()
{
    VkResult result;

    QueueFamilyIndices indices = p_device->GetQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };
    assert(indices.isComplete());

    VkSwapchainCreateInfoKHR createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = p_surface->GetSurface();
        createInfo.minImageCount = m_imageCount;
        createInfo.imageFormat = m_surfaceFormat.format;
        createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
        createInfo.imageExtent = m_extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = m_currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
    }

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
        createInfo.format = m_surfaceFormat.format;
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

void SwapChain::CreateDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    p_device->CreateImage(m_extent.width, m_extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = p_device->CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat SwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(p_device->GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}
