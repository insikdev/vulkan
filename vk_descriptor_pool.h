#pragma once

class Device;

class DescriptorPool {
public:
    DescriptorPool(const Device*, const std::vector<VkDescriptorPoolSize>&, uint32_t maxSets);
    ~DescriptorPool();

public:
    VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout&);

public: // getter
    VkDescriptorPool GetPool() { return m_pool; }

private:
    void CreatePool(const std::vector<VkDescriptorPoolSize>&, uint32_t maxSets);

private:
    const Device* p_device;

private:
    VkDescriptorPool m_pool;
};
