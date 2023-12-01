#include "pch.h"
#include "vk_command_pool.h"
#include "vk_device.h"
#include "vk_command_buffer.h"

CommandPool::CommandPool(const Device* pDevice)
    : p_device { pDevice }
{
    CreatePool();
}

CommandPool::~CommandPool()
{
    // All allocated VkCommandBuffer will be implicitly destroyed when the VkCommandPool is destroyed.
    vkDestroyCommandPool(p_device->GetDevice(), m_pool, nullptr);
}

CommandBuffer CommandPool::AllocateCommandBuffer()
{
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
    }

    VkResult result = vkAllocateCommandBuffers(p_device->GetDevice(), &allocInfo, &commandBuffer);
    CHECK_VK(result);

    return CommandBuffer { p_device, commandBuffer };
}

void CommandPool::FreeCommandBuffer(CommandBuffer& commandBuffer)
{
    vkFreeCommandBuffers(p_device->GetDevice(), m_pool, 1, commandBuffer.GetPtr());
}

void CommandPool::CreatePool()
{
    VkCommandPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = p_device->GetQueueFamilyIndices().graphicsFamily;
    }

    VkResult result = vkCreateCommandPool(p_device->GetDevice(), &poolInfo, nullptr, &m_pool);
    CHECK_VK(result);
}
