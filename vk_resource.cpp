#include "pch.h"
#include "vk_resource.h"
#include "vk_device.h"
#include "vk_command_pool.h"
#include "vk_command_buffer.h"

uint32_t Resource::s_count = 0;
CommandPool* Resource::s_pool = nullptr;

Resource::Resource(const Device* pDevice)
    : p_device { pDevice }
{
    if (s_count == 0) {
        s_pool = new CommandPool(pDevice);
    }
    s_count++;
}

Resource::~Resource()
{
    s_count--;
    if (s_count == 0) {
        delete s_pool;
    }

    vkFreeMemory(p_device->GetDevice(), m_deviceMemory, nullptr);
}

CommandBuffer Resource::BeginSingleTimeCommand(void)
{
    CommandBuffer commandBuffer = s_pool->AllocateCommandBuffer();
    commandBuffer.Begin();

    return commandBuffer;
}

void Resource::EndSingleTimeCommand(CommandBuffer commandBuffer)
{
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
