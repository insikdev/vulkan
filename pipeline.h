#pragma once

#include "pch.h"

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

public: // getter
    inline VkPipelineLayout GetPipelineLayout() const { return m_layout; }
    inline VkRenderPass GetRenderPass() const { return m_renderPass; }
    inline VkPipeline GetPipeline() const { return m_pipeline; }

private:
    void CreatePipelineLayout();
    void CreateRenderPass();
    void CreatePipeline();

private:
    const Device* p_device;
    const SwapChain* p_swapChain;

private:
    VkPipelineLayout m_layout;
    VkRenderPass m_renderPass;
    VkPipeline m_pipeline;
};
