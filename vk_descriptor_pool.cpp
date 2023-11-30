#include "pch.h"
#include "vk_descriptor_pool.h"
#include "vk_device.h"

DescriptorPool::DescriptorPool(const Device* pDevice, const std::vector<VkDescriptorPoolSize>& poolSizes)
    : p_device { pDevice }
{
    CreatePool(poolSizes);
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(p_device->GetDevice(), m_pool, nullptr);
}

void DescriptorPool::CreatePool(const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 10;
    }

    VkResult result = vkCreateDescriptorPool(p_device->GetDevice(), &poolInfo, nullptr, &m_pool);
    CHECK_VK(result);
}
