#include "pch.h"
#include "vk_buffer.h"
#include "vk_device.h"
#include "vk_command_pool.h"
#include "vk_command_buffer.h"
#include "query.h"

uint32_t Buffer::s_count = 0;
CommandPool* Buffer::s_pool = nullptr;

Buffer::Buffer(const Device* pDevice, VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memoryPropertyFlags)
    : p_device { pDevice }
{
    if (s_count == 0) {
        s_pool = new CommandPool(pDevice);
    }

    CreateBuffer(createInfo, memoryPropertyFlags);
    s_count++;
}

Buffer::~Buffer()
{
    vkDestroyBuffer(p_device->GetDevice(), m_buffer, nullptr);
    vkFreeMemory(p_device->GetDevice(), m_deviceMemory, nullptr);

    s_count--;
    if (s_count == 0) {
        delete s_pool;
    }
}

void Buffer::CreateBuffer(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memoryPropertyFlags)
{
    m_size = createInfo.size;

    VkResult result = vkCreateBuffer(p_device->GetDevice(), &createInfo, nullptr, &m_buffer);
    CHECK_VK(result);

    VkMemoryRequirements memRequirements {};
    vkGetBufferMemoryRequirements(p_device->GetDevice(), m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = p_device->FindMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);
    }

    result = vkAllocateMemory(p_device->GetDevice(), &allocInfo, nullptr, &m_deviceMemory);
    CHECK_VK(result);

    vkBindBufferMemory(p_device->GetDevice(), m_buffer, m_deviceMemory, 0);
}

void Buffer::Copy(VkBuffer srcBuffer)
{
    CommandBuffer commandBuffer = s_pool->AllocateCommandBuffer();
    commandBuffer.Begin();

    VkBufferCopy copyRegion {};
    {
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = m_size;
    }

    vkCmdCopyBuffer(commandBuffer.GetHandle(), srcBuffer, m_buffer, 1, &copyRegion);
    commandBuffer.End();

    VkSubmitInfo submitInfo {};
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer.GetPtr();
    }

    vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(p_device->GetQueue());
    s_pool->FreeCommandBuffer(commandBuffer);
}

void Buffer::MapMemory()
{
    vkMapMemory(p_device->GetDevice(), m_deviceMemory, 0, m_size, 0, &p_host);
}

void Buffer::UnmapMemory()
{
    vkUnmapMemory(p_device->GetDevice(), m_deviceMemory);
    p_host = nullptr;
}

void Buffer::InvalidateMappedMemory(void)
{
    VkMappedMemoryRange mappedMemoryRange {};
    {
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = m_deviceMemory;
        mappedMemoryRange.offset = 0;
        mappedMemoryRange.size = m_size;
    }

    VkResult result = vkInvalidateMappedMemoryRanges(p_device->GetDevice(), 1, &mappedMemoryRange);
    CHECK_VK(result);
}

void Buffer::FlushMappedMemory(void)
{
    VkMappedMemoryRange mappedMemoryRange {};
    {
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = m_deviceMemory;
        mappedMemoryRange.offset = 0;
        mappedMemoryRange.size = m_size;
    }

    VkResult result = vkFlushMappedMemoryRanges(p_device->GetDevice(), 1, &mappedMemoryRange);
    CHECK_VK(result);
}
