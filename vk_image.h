#pragma once

#include "vk_resource.h"
class Device;

class Image : public Resource {
public:
    Image() = delete;
    Image(const Device*, VkImageCreateInfo);
    Image(const Device*, std::string);
    ~Image();

public:
    void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
    VkImageView CreateImageView(VkFormat);

public: // getter
    VkImage GetImage(void) { return m_image; }
    // VkImageView GetImageView(void) { return m_view; }

private:
    void CreateImage(VkImageCreateInfo);
    void CreateTextureImage(std::string);

private:
    VkImage m_image;
    // VkImageView m_view;
};
