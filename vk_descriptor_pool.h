#pragma once

class Device;

class DescriptorPool {
public:
    DescriptorPool(const Device*, const std::vector<VkDescriptorPoolSize>&);
    ~DescriptorPool();

public:
    VkDescriptorSet AllocateDescriptorSet();

public: // getter
    VkDescriptorPool GetPool() { return m_pool; }

private:
    void CreatePool(const std::vector<VkDescriptorPoolSize>&);

private:
    const Device* p_device;

private:
    VkDescriptorPool m_pool;
};
