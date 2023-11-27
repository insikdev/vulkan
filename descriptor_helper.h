#pragma once

class Device;

class DescriptorHelper {
public:
    DescriptorHelper(const Device*, VkDescriptorPoolSize, uint32_t);
    ~DescriptorHelper();

public:
    VkDescriptorSet AllocateDescriptorSet(const std::vector<VkDescriptorSetLayout>&) const;
    void UpdateDescriptorSet(VkWriteDescriptorSet) const;

private:
    const Device* p_device;

private:
    void CreateDescriptorPool();

private:
    VkDescriptorPoolSize m_poolSize;
    uint32_t m_maxSets;
    VkDescriptorPool m_descriptorPool;
};
