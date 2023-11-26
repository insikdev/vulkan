#include "pch.h"
#include "pipeline.h"
#include "device.h"
#include "swap_chain.h"
#include "shader.h"

Pipeline::Pipeline(const Device* pDevice, const SwapChain* pSwapChain)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
{
    CreatePipelineLayout();
    CreateRenderPass();
    CreatePipeline();
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(p_device->GetDevice(), m_pipeline, nullptr);
    vkDestroyRenderPass(p_device->GetDevice(), m_renderPass, nullptr);
    vkDestroyPipelineLayout(p_device->GetDevice(), m_layout, nullptr);
}

void Pipeline::UpdateSwapChain(const SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;
    vkDestroyRenderPass(p_device->GetDevice(), m_renderPass, nullptr);
    CreateRenderPass();
}

void Pipeline::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pushConstantRangeCount = 0;
        createInfo.setLayoutCount = 0;
    }

    VkResult result = vkCreatePipelineLayout(p_device->GetDevice(), &createInfo, nullptr, &m_layout);
    CHECK_VK(result);
}

void Pipeline::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment {};
    {
        colorAttachment.format = p_swapChain->GetFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkAttachmentReference colorAttachmentRef = {};
    {
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
    }

    VkSubpassDependency dependency {};
    {
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    VkRenderPassCreateInfo createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &colorAttachment;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
    }

    VkResult result = vkCreateRenderPass(p_device->GetDevice(), &createInfo, nullptr, &m_renderPass);
    CHECK_VK(result);
}

void Pipeline::CreatePipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkShaderModule vertexShader;
    Shader::CreateModule(p_device->GetDevice(), "shaders/simple.vert.spv", &vertexShader);

    VkPipelineShaderStageCreateInfo vertexShaderStage {};
    {
        vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStage.module = vertexShader;
        vertexShaderStage.pName = "main";
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    shaderStages.push_back(vertexShaderStage);

    VkShaderModule fragmentShader;
    Shader::CreateModule(p_device->GetDevice(), "shaders/simple.frag.spv", &fragmentShader);

    VkPipelineShaderStageCreateInfo fragmentShaderStage {};
    {
        fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStage.module = fragmentShader;
        fragmentShaderStage.pName = "main";
        fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    shaderStages.push_back(fragmentShaderStage);

    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    auto bindingDescriptions = Vertex::GetBindingDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputState {};
    {
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {};
    {
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    VkPipelineTessellationStateCreateInfo tessellationState {};
    {
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    }

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.width = static_cast<float>(p_swapChain->GetExtent2D().width);
    viewport.height = static_cast<float>(p_swapChain->GetExtent2D().height);

    VkRect2D scissor {};
    scissor.extent = p_swapChain->GetExtent2D();
    scissor.offset = { 0, 0 };

    VkPipelineViewportStateCreateInfo viewportState {};
    {
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
    }

    VkPipelineRasterizationStateCreateInfo rasterizationState {};
    {
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState.lineWidth = 1.0f;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    }

    VkPipelineMultisampleStateCreateInfo multisampleState {};
    {
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    VkPipelineDepthStencilStateCreateInfo depthStencilState {};
    {
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState {};
    {
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &colorBlendAttachment;
        colorBlendState.blendConstants[0] = 0.0f;
        colorBlendState.blendConstants[1] = 0.0f;
        colorBlendState.blendConstants[2] = 0.0f;
        colorBlendState.blendConstants[3] = 0.0f;
    }

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState {};
    {
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
    }

    VkGraphicsPipelineCreateInfo createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        createInfo.pStages = shaderStages.data();
        createInfo.pColorBlendState = &colorBlendState;
        createInfo.pDepthStencilState = &depthStencilState;
        createInfo.pDynamicState = &dynamicState;
        createInfo.pInputAssemblyState = &inputAssemblyState;
        createInfo.pMultisampleState = &multisampleState;
        createInfo.pRasterizationState = &rasterizationState;
        createInfo.pTessellationState = &tessellationState;
        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pViewportState = &viewportState;

        createInfo.layout = m_layout;
        createInfo.renderPass = m_renderPass;
        createInfo.subpass = 0;
    }

    VkResult result = vkCreateGraphicsPipelines(p_device->GetDevice(), nullptr, 1, &createInfo, nullptr, &m_pipeline);
    CHECK_VK(result);

    vkDestroyShaderModule(p_device->GetDevice(), vertexShader, nullptr);
    vkDestroyShaderModule(p_device->GetDevice(), fragmentShader, nullptr);
}
