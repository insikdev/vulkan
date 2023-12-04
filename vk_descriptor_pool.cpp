#include "pch.h"
#include "vk_descriptor_pool.h"
#include "vk_device.h"

DescriptorPool::DescriptorPool(const Device* pDevice, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
    : p_device { pDevice }
{
    CreatePool(poolSizes, maxSets);
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(p_device->GetDevice(), m_pool, nullptr);
}

VkDescriptorSet DescriptorPool::AllocateDescriptorSet(const std::vector<VkDescriptorSetLayout>& layouts)
{
    VkDescriptorSetAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();
    }

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(p_device->GetDevice(), &allocInfo, &descriptorSet);

    return descriptorSet;
}

void DescriptorPool::CreatePool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
    }

    VkResult result = vkCreateDescriptorPool(p_device->GetDevice(), &poolInfo, nullptr, &m_pool);
    CHECK_VK(result);
}
