#include "pch.h"
#include "renderer.h"
#include "vk_device.h"
#include "vk_swap_chain.h"
#include "vk_pipeline.h"
#include "scene.h"
#include "model.h"
#include "camera.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "vk_command_pool.h"
#include "vk_command_buffer.h"
#include "vk_buffer.h"
#include "vk_descriptor_pool.h"
#include "vk_image.h"

Renderer::Renderer(Device* pDevice, SwapChain* pSwapChain, const Pipeline* pPipeline)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
    , p_pipeline { pPipeline }
{
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
    InitPerFrame();
    CreateTextureImage();
    CreateSampler();
    CreateCommonUniform();
}

Renderer::~Renderer()
{
    vkDestroySampler(p_device->GetDevice(), textureSampler, nullptr);
    vkDestroyImageView(p_device->GetDevice(), imageView, nullptr);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        delete m_frames[i].p_commandPool;
        vkDestroySemaphore(p_device->GetDevice(), m_frames[i].imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(p_device->GetDevice(), m_frames[i].renderFinishedSemaphore, nullptr);
        vkDestroyFence(p_device->GetDevice(), m_frames[i].inFlightFence, nullptr);
    }

    m_uniform->UnmapMemory();
    delete m_uniform;
    delete p_image;
    delete p_descriptorPool;
}

void Renderer::SetScene(Scene* pScene)
{
    p_scene = pScene;

    if (p_descriptorPool != nullptr) {
        delete p_descriptorPool;
    }

    uint32_t numModels = static_cast<uint32_t>(p_scene->GetModels().size());

    std::vector<VkDescriptorPoolSize> poolSizes {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numModels + 1 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numModels },
    };

    p_descriptorPool = new DescriptorPool { p_device, poolSizes, numModels + 1 };

    for (auto& model : p_scene->GetModels()) {
        VkDescriptorSet dstSet = p_descriptorPool->AllocateDescriptorSet(p_pipeline->GetModelDescriptorSetLayouts());
        model->SetDescriptorSet(dstSet, imageView, textureSampler);
    }

    m_commonDescriptorSet = p_descriptorPool->AllocateDescriptorSet(p_pipeline->GetCommonDescriptorSetLayouts());

    VkDescriptorBufferInfo bufferInfo {};
    {
        bufferInfo.buffer = m_uniform->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CommonUniform);
    }

    VkWriteDescriptorSet uniformDS {};
    {
        uniformDS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformDS.dstSet = m_commonDescriptorSet;
        uniformDS.dstBinding = 0;
        uniformDS.dstArrayElement = 0;
        uniformDS.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformDS.descriptorCount = 1;
        uniformDS.pBufferInfo = &bufferInfo;
    }

    vkUpdateDescriptorSets(p_device->GetDevice(), 1, &uniformDS, 0, nullptr);
}

void Renderer::Update(float dt)
{
    CommonUniform uniformData {};
    uniformData.view = p_scene->p_camera->GetViewMatrix();
    uniformData.proj = p_scene->p_camera->GetProjectionMatrix();
    uniformData.eyePos = p_scene->p_camera->m_position;
    uniformData.lightPos = p_scene->lightPos;
    uniformData.lightDir = p_scene->lightDir;
    uniformData.lightColor = p_scene->lightColor;

    m_uniform->InvalidateMappedMemory();
    memcpy(m_uniform->GetMappedPtr(), &uniformData, sizeof(CommonUniform));
    m_uniform->FlushMappedMemory();

    for (auto& model : p_scene->GetModels()) {
        model->Update(dt);
    }
}

void Renderer::Render()
{
    vkWaitForFences(p_device->GetDevice(), 1, &m_frames[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, m_frames[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    if (result != VK_SUBOPTIMAL_KHR) {
        CHECK_VK(result);
    }

    vkResetFences(p_device->GetDevice(), 1, &m_frames[currentFrame].inFlightFence);

    VkCommandBuffer commandBuffer = m_frames[currentFrame].commandBuffer;

    RecordCommandBuffer(commandBuffer, imageIndex);

    VkSemaphore waitSemaphores[] = { m_frames[currentFrame].imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_frames[currentFrame].renderFinishedSemaphore;
    }

    CHECK_VK(vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, m_frames[currentFrame].inFlightFence));

    VkSwapchainKHR swapChains[] = { p_swapChain->GetSwapChain() };
    VkPresentInfoKHR presentInfo { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_frames[currentFrame].renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
    }

    result = vkQueuePresentKHR(p_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    CHECK_VK(result);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::UpdateSwapChain(SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
}

void Renderer::CreateCommonUniform()
{
    VkBufferCreateInfo createInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    {
        createInfo.size = sizeof(CommonUniform);
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    m_uniform = new Buffer { p_device, createInfo, memoryPropertyFlags };
    m_uniform->MapMemory();
}

void Renderer::InitPerFrame()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandPool* cmdPool = new CommandPool { p_device };
        CommandBuffer cmdBuffer = cmdPool->AllocateCommandBuffer();

        m_frames[i].p_commandPool = cmdPool;
        m_frames[i].commandBuffer = cmdBuffer.GetHandle();

        VkSemaphoreCreateInfo semaphoreInfo {};
        {
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        }

        VkFenceCreateInfo fenceInfo {};
        {
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        CHECK_VK(vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &m_frames[i].imageAvailableSemaphore));
        CHECK_VK(vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &m_frames[i].renderFinishedSemaphore));
        CHECK_VK(vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &m_frames[i].inFlightFence));
    }
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    CHECK_VK(result);
    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo {};
    {
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = p_pipeline->GetRenderPass();
        renderPassInfo.framebuffer = p_swapChain->GetFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = p_swapChain->GetExtent2D();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    }

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipeline());

    VkViewport viewport {};
    {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(p_swapChain->GetExtent2D().width);
        viewport.height = static_cast<float>(p_swapChain->GetExtent2D().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
    }
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    {
        scissor.offset = { 0, 0 };
        scissor.extent = p_swapChain->GetExtent2D();
    }
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    /* auto matrix = p_scene->GetViewProjMatrix();
     vkCmdPushConstants(commandBuffer, p_pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CameraUniform), &matrix);*/

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipelineLayout(), 1, 1, &m_commonDescriptorSet, 0, nullptr);

    for (int i = 0; i < p_scene->GetModels().size(); i++) {
        Model* model = p_scene->GetModels()[i];
        model->Bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipelineLayout(), 0, 1, &model->m_descriptorSet, 0, nullptr);
        model->Draw(commandBuffer);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Settings")) {
        ImGui::Text("Frame Time : %.2f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("Camera");
        ImGui::SliderFloat("x", &p_scene->p_camera->m_position.x, -10.0f, 10.0f);
        ImGui::SliderFloat("y", &p_scene->p_camera->m_position.y, -10.0f, 10.0f);
        ImGui::SliderFloat("z", &p_scene->p_camera->m_position.z, -10.0f, 10.0f);
        ImGui::Separator();

        ImGui::Text("Light");
        ImGui::SliderFloat("lx", &p_scene->lightPos.x, -10.0f, 10.0f);
        ImGui::SliderFloat("ly", &p_scene->lightPos.y, -10.0f, 10.0f);
        ImGui::SliderFloat("lz", &p_scene->lightPos.z, -10.0f, 10.0f);
        ImGui::Separator();

        ImGui::Text("Model Settings");
        ImGui::SliderFloat("rx", &p_scene->GetModels()[0]->m_transform.m_rotation.x, -100.0f, 100.0f);
        ImGui::SliderFloat("ry", &p_scene->GetModels()[0]->m_transform.m_rotation.y, -100.0f, 100.0f);
        ImGui::SliderFloat("rz", &p_scene->GetModels()[0]->m_transform.m_rotation.z, -100.0f, 100.0f);

        ImGui::SliderFloat("ar", &p_scene->GetModels()[0]->ambient.r, 0.0f, 1.0f);
        ImGui::SliderFloat("ag", &p_scene->GetModels()[0]->ambient.g, 0.0f, 1.0f);
        ImGui::SliderFloat("ab", &p_scene->GetModels()[0]->ambient.b, 0.0f, 1.0f);

        ImGui::SliderFloat("dr", &p_scene->GetModels()[0]->diffuse.r, 0.0f, 1.0f);
        ImGui::SliderFloat("dg", &p_scene->GetModels()[0]->diffuse.g, 0.0f, 1.0f);
        ImGui::SliderFloat("db", &p_scene->GetModels()[0]->diffuse.b, 0.0f, 1.0f);

        ImGui::SliderFloat("s", &p_scene->GetModels()[0]->shininess, 1.0f, 1000.0f);

        ImGui::Separator();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VK(result);
}

void Renderer::CreateTextureImage()
{
    // p_image = new Image { p_device, "textures/smile.png" };
    p_image = new Image { p_device, "textures/sample.jpg" };

    imageView = p_image->CreateImageView(VK_FORMAT_R8G8B8A8_SRGB);
}

void Renderer::CreateSampler()
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(p_device->GetPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(p_device->GetDevice(), &samplerInfo, nullptr, &textureSampler);
}
