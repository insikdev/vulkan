#pragma once

class Device;
class SwapChain;

class Pipeline {
public:
    Pipeline(const Device*, const SwapChain*);
    ~Pipeline();
    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

public:
    void UpdateSwapChain(const SwapChain*);

public: // getter
    inline std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }
    inline VkPipelineLayout GetPipelineLayout() const { return m_layout; }
    inline VkRenderPass GetRenderPass() const { return m_renderPass; }
    inline VkPipeline GetPipeline() const { return m_pipeline; }

private:
    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreateRenderPass();
    void CreatePipeline();

private:
    const Device* p_device;
    const SwapChain* p_swapChain;

private:
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    VkPipelineLayout m_layout;
    VkRenderPass m_renderPass;
    VkPipeline m_pipeline;
};
