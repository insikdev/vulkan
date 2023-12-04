#include "pch.h"
#include "vk_image.h"
#include "vk_device.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "vk_buffer.h"
#include "vk_command_buffer.h"

Image::Image(const Device* pDevice, VkImageCreateInfo createInfo)
    : Resource { pDevice }
{
    CreateImage(createInfo);
}

Image::Image(const Device* pDevice, std::string filename)
    : Resource { pDevice }
{
    CreateTextureImage(filename);
}

Image::~Image()
{
    // if (m_view != VK_NULL_HANDLE) {
    //     vkDestroyImageView(p_device->GetDevice(), m_view, nullptr);
    // }

    vkDestroyImage(p_device->GetDevice(), m_image, nullptr);
}

void Image::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    CommandBuffer commandBuffer = BeginSingleTimeCommand();

    VkImageMemoryBarrier barrier {};
    {
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer.GetHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommand(commandBuffer);
}

void Image::copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height)
{
    CommandBuffer commandBuffer = BeginSingleTimeCommand();

    VkBufferImageCopy region {};
    {
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };
    }

    vkCmdCopyBufferToImage(commandBuffer.GetHandle(), buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommand(commandBuffer);
}

VkImageView Image::CreateImageView(VkFormat format)
{
    VkImageViewCreateInfo viewInfo {};
    {
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
    }

    VkImageView imageView;
    VkResult result = vkCreateImageView(p_device->GetDevice(), &viewInfo, nullptr, &imageView);
    CHECK_VK(result);

    return imageView;
}

void Image::CreateImage(VkImageCreateInfo createInfo)
{
    VkResult result = vkCreateImage(p_device->GetDevice(), &createInfo, nullptr, &m_image);
    CHECK_VK(result);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(p_device->GetDevice(), m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = p_device->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    result = vkAllocateMemory(p_device->GetDevice(), &allocInfo, nullptr, &m_deviceMemory);
    CHECK_VK(result);

    result = vkBindImageMemory(p_device->GetDevice(), m_image, m_deviceMemory, 0);
    CHECK_VK(result);
}

void Image::CreateTextureImage(std::string filename)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBufferCreateInfo stagingBufferCreateInfo {};
    {
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.size = imageSize;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkMemoryPropertyFlags stagingBufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer { p_device, stagingBufferCreateInfo, stagingBufferMemoryPropertyFlags };
    stagingBuffer.MapMemory();
    memcpy(stagingBuffer.GetMappedPtr(), pixels, imageSize);
    stagingBuffer.UnmapMemory();

    stbi_image_free(pixels);

    VkImageCreateInfo imageCreateInfo {};
    {
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageCreateInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.flags = 0; // Optional
    }

    CreateImage(imageCreateInfo);

    transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer.GetBuffer(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
