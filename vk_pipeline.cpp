#include "pch.h"
#include "vk_pipeline.h"
#include "vk_device.h"
#include "vk_swap_chain.h"
#include "shader.h"

Pipeline::Pipeline(const Device* pDevice, const SwapChain* pSwapChain)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
{
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreateRenderPass();
    CreatePipeline();
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(p_device->GetDevice(), m_pipeline, nullptr);
    vkDestroyRenderPass(p_device->GetDevice(), m_renderPass, nullptr);
    vkDestroyPipelineLayout(p_device->GetDevice(), m_layout, nullptr);

    for (const auto& layout : m_descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(p_device->GetDevice(), layout, nullptr);
    }
}

void Pipeline::UpdateSwapChain(const SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;
    vkDestroyRenderPass(p_device->GetDevice(), m_renderPass, nullptr);
    CreateRenderPass();
}

void Pipeline::CreateDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 2> modelDescriptorSetLayoutBinding {};
    {
        modelDescriptorSetLayoutBinding[0].binding = 0;
        modelDescriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelDescriptorSetLayoutBinding[0].descriptorCount = 1;
        modelDescriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

        modelDescriptorSetLayoutBinding[1].binding = 1;
        modelDescriptorSetLayoutBinding[1].descriptorCount = 1;
        modelDescriptorSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        modelDescriptorSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    }

    VkDescriptorSetLayoutCreateInfo modelDescriptorSetLayoutCreateInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    {
        modelDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(modelDescriptorSetLayoutBinding.size());
        modelDescriptorSetLayoutCreateInfo.pBindings = modelDescriptorSetLayoutBinding.data();
    }

    VkDescriptorSetLayout modelDescriptorSetLayout;
    CHECK_VK(vkCreateDescriptorSetLayout(p_device->GetDevice(), &modelDescriptorSetLayoutCreateInfo, nullptr, &modelDescriptorSetLayout));
    m_descriptorSetLayouts.push_back(modelDescriptorSetLayout);

    std::array<VkDescriptorSetLayoutBinding, 1> commonDescriptorSetLayoutBinding {};
    {
        commonDescriptorSetLayoutBinding[0].binding = 0;
        commonDescriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        commonDescriptorSetLayoutBinding[0].descriptorCount = 1;
        commonDescriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    }

    VkDescriptorSetLayoutCreateInfo commonDescriptorSetLayoutCreateInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    {
        commonDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(commonDescriptorSetLayoutBinding.size());
        commonDescriptorSetLayoutCreateInfo.pBindings = commonDescriptorSetLayoutBinding.data();
    }

    VkDescriptorSetLayout commonDescriptorSetLayout;
    CHECK_VK(vkCreateDescriptorSetLayout(p_device->GetDevice(), &commonDescriptorSetLayoutCreateInfo, nullptr, &commonDescriptorSetLayout));
    m_descriptorSetLayouts.push_back(commonDescriptorSetLayout);
}

void Pipeline::CreatePipelineLayout()
{
    // VkPushConstantRange pushConstant {};
    //{
    //     pushConstant.size = sizeof(Mat4);
    //     pushConstant.offset = 0;
    //     pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    // }

    VkPipelineLayoutCreateInfo createInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    {
        createInfo.pushConstantRangeCount = 0;
        // createInfo.pushConstantRangeCount = 1;
        // createInfo.pPushConstantRanges = &pushConstant;
        createInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
        createInfo.pSetLayouts = m_descriptorSetLayouts.data();
    }

    CHECK_VK(vkCreatePipelineLayout(p_device->GetDevice(), &createInfo, nullptr, &m_layout));
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

    VkAttachmentDescription depthAttachment {};
    {
        depthAttachment.format = p_swapChain->findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depthAttachmentRef {};
    {
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }

    VkSubpassDependency dependency {};
    {
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo createInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    {
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
    }

    CHECK_VK(vkCreateRenderPass(p_device->GetDevice(), &createInfo, nullptr, &m_renderPass));
}

void Pipeline::CreatePipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkShaderModule vertexShader;
    Shader::CreateModule(p_device->GetDevice(), "shaders/simple.vert.spv", &vertexShader);

    VkPipelineShaderStageCreateInfo vertexShaderStage { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    {
        vertexShaderStage.module = vertexShader;
        vertexShaderStage.pName = "main";
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    shaderStages.push_back(vertexShaderStage);

    VkShaderModule fragmentShader;
    Shader::CreateModule(p_device->GetDevice(), "shaders/simple.frag.spv", &fragmentShader);

    VkPipelineShaderStageCreateInfo fragmentShaderStage { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    {
        fragmentShaderStage.module = fragmentShader;
        fragmentShaderStage.pName = "main";
        fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    shaderStages.push_back(fragmentShaderStage);

    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    auto bindingDescriptions = Vertex::GetBindingDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputState { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    {
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    {
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    VkPipelineTessellationStateCreateInfo tessellationState { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };

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

    VkPipelineViewportStateCreateInfo viewportState { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    {
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
    }

    VkPipelineRasterizationStateCreateInfo rasterizationState { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    {
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.lineWidth = 1.0f;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    }

    VkPipelineMultisampleStateCreateInfo multisampleState { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    {
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    VkPipelineDepthStencilStateCreateInfo depthStencilState { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    {
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.minDepthBounds = 0.0f; // Optional
        depthStencilState.maxDepthBounds = 1.0f; // Optional
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {}; // Optional
        depthStencilState.back = {}; // Optional
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    {
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

    VkPipelineDynamicStateCreateInfo dynamicState { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    {
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
    }

    VkGraphicsPipelineCreateInfo createInfo { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    {
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

    CHECK_VK(vkCreateGraphicsPipelines(p_device->GetDevice(), nullptr, 1, &createInfo, nullptr, &m_pipeline));

    vkDestroyShaderModule(p_device->GetDevice(), vertexShader, nullptr);
    vkDestroyShaderModule(p_device->GetDevice(), fragmentShader, nullptr);
}
