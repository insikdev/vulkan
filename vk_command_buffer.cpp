#include "pch.h"
#include "vk_command_buffer.h"

CommandBuffer::CommandBuffer(const Device* pDevice, VkCommandBuffer commandBuffer)
    : p_device { pDevice }
    , m_commandBuffer { commandBuffer }
{
}

void CommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    CHECK_VK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::End()
{
    CHECK_VK(vkEndCommandBuffer(m_commandBuffer));
}
