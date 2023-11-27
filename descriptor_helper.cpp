#include "pch.h"
#include "descriptor_helper.h"
#include "device.h"

DescriptorHelper::DescriptorHelper(const Device* pDevice, VkDescriptorPoolSize poolSize, uint32_t maxSets)
    : p_device { pDevice }
    , m_poolSize { poolSize }
    , m_maxSets { maxSets }
{
    CreateDescriptorPool();
}

DescriptorHelper::~DescriptorHelper()
{
    /*
     * VkDescriptorSet will be implicitly destroyed when the VkDescriptorPool is destroyed.
     */
    vkDestroyDescriptorPool(p_device->GetDevice(), m_descriptorPool, nullptr);
}

VkDescriptorSet DescriptorHelper::AllocateDescriptorSet(const std::vector<VkDescriptorSetLayout>& layouts) const
{
    VkDescriptorSetAllocateInfo allocInfo {};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();
    }

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(p_device->GetDevice(), &allocInfo, &descriptorSet);

    return descriptorSet;
}

void DescriptorHelper::UpdateDescriptorSet(VkWriteDescriptorSet descriptorWrite) const
{
    vkUpdateDescriptorSets(p_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

void DescriptorHelper::CreateDescriptorPool()
{
    VkDescriptorPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &m_poolSize;
        poolInfo.maxSets = m_maxSets;
    }

    VkResult result = vkCreateDescriptorPool(p_device->GetDevice(), &poolInfo, nullptr, &m_descriptorPool);
    CHECK_VK(result);
}
