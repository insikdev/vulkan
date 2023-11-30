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

Renderer::Renderer(Device* pDevice, SwapChain* pSwapChain, const Pipeline* pPipeline)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
    , p_pipeline { pPipeline }
{
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
    p_commandPool = new CommandPool { p_device };
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_commandBuffers.push_back(p_commandPool->AllocateCommandBuffer());
    }

    // p_device->AllocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
    CreateSyncObjects();
    createDescriptorPool();
}

Renderer::~Renderer()
{

    vkDestroyDescriptorPool(p_device->GetDevice(), m_descriptorPool, nullptr);

    // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //     vkDestroyBuffer(p_device->GetDevice(), uniformBuffers[i], nullptr);
    //     vkFreeMemory(p_device->GetDevice(), uniformBuffersMemory[i], nullptr);
    // }

    // for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //     vkDestroySemaphore(p_device->GetDevice(), imageAvailableSemaphores[i], nullptr);
    //     vkDestroySemaphore(p_device->GetDevice(), renderFinishedSemaphores[i], nullptr);
    //     vkDestroyFence(p_device->GetDevice(), inFlightFences[i], nullptr);
    // }

    vkDestroySemaphore(p_device->GetDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(p_device->GetDevice(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(p_device->GetDevice(), inFlightFence, nullptr);

    delete p_commandPool;
}

void Renderer::Update(const Scene* pScene, float dt)
{
    for (auto& model : pScene->GetModels()) {
        const_cast<Model*>(model)->Update(dt);
    }
}

void Renderer::Render(const Scene* pScene)
{
    // vkWaitForFences(p_device->GetDevice(), 1, &inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkWaitForFences(p_device->GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    // VkResult result = vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    VkResult result = vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    if (result != VK_SUBOPTIMAL_KHR) {
        CHECK_VK(result);
    }

    // vkResetFences(p_device->GetDevice(), 1, &inFlightFences[m_currentFrame]);
    vkResetFences(p_device->GetDevice(), 1, &inFlightFence);

    // VkCommandBuffer commandBuffer = p_command->GetCommandBuffer(m_currentFrame);
    VkCommandBuffer commandBuffer = m_commandBuffers[0].GetHandle();

    RecordCommandBuffer(commandBuffer, imageIndex, pScene);

    // VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[m_currentFrame] };
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo {};
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores = &renderFinishedSemaphores[m_currentFrame];
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
    }

    // result = vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, inFlightFences[m_currentFrame]);
    result = vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, inFlightFence);
    CHECK_VK(result);

    VkSwapchainKHR swapChains[] = { p_swapChain->GetSwapChain() };
    VkPresentInfoKHR presentInfo {};
    {
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        // presentInfo.pWaitSemaphores = &renderFinishedSemaphores[m_currentFrame];
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
    }

    result = vkQueuePresentKHR(p_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    CHECK_VK(result);

    // m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::UpdateSwapChain(SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
}

// void Renderer::createDescriptorSets()
//{
//     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//         auto descriptorSet = p_descriptor->AllocateDescriptorSet(p_pipeline->GetDescriptorSetLayouts());
//         descriptorSets.push_back(descriptorSet);
//     }
//
//     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//         VkDescriptorBufferInfo bufferInfo {};
//         {
//             bufferInfo.buffer = uniformBuffers[i];
//             bufferInfo.offset = 0;
//             bufferInfo.range = sizeof(UniformBufferObject);
//         }
//
//         VkWriteDescriptorSet descriptorWrite {};
//         {
//             descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             descriptorWrite.dstSet = descriptorSets[i];
//             descriptorWrite.dstBinding = 0;
//             descriptorWrite.dstArrayElement = 0;
//             descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//             descriptorWrite.descriptorCount = 1;
//             descriptorWrite.pBufferInfo = &bufferInfo;
//             descriptorWrite.pImageInfo = nullptr; // Optional
//             descriptorWrite.pTexelBufferView = nullptr; // Optional
//         }
//
//         p_descriptor->UpdateDescriptorSet(descriptorWrite);
//     }
// }

VkDescriptorSet Renderer::createDescriptorSet(std::vector<VkDescriptorSetLayout> layouts)
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

void Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes {};
    {
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 10;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 10;
    }

    VkDescriptorPoolCreateInfo poolInfo {};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1000;
    }

    VkResult result = vkCreateDescriptorPool(p_device->GetDevice(), &poolInfo, nullptr, &m_descriptorPool);
    CHECK_VK(result);
}

void Renderer::CreateSyncObjects()
{
    VkResult result;
    VkSemaphoreCreateInfo semaphoreInfo {};
    {
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    }

    VkFenceCreateInfo fenceInfo {};
    {
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    // for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //     result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
    //     CHECK_VK(result);
    //     result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
    //     CHECK_VK(result);
    //     result = vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]);
    //     CHECK_VK(result);
    // }

    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    CHECK_VK(result);
    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
    CHECK_VK(result);
    result = vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &inFlightFence);
    CHECK_VK(result);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene* pScene)
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

    vkResetDescriptorPool(p_device->GetDevice(), m_descriptorPool, 0);

    auto matrix = pScene->GetViewProjMatrix();
    vkCmdPushConstants(commandBuffer, p_pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CameraUniform), &matrix);

    for (const auto& model : pScene->GetModels()) {

        model->Bind(commandBuffer);

        auto dstSet = createDescriptorSet(p_pipeline->GetDescriptorSetLayouts());
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = model->m_uniform->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUniform);

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = dstSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(p_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipelineLayout(), 0, 1, &dstSet, 0, nullptr);

        model->Draw(commandBuffer);
    }
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera");

    ImGui::SliderFloat("x", &pScene->p_camera->m_position.x, -10.0f, 10.0f);
    ImGui::SliderFloat("y", &pScene->p_camera->m_position.y, -10.0f, 10.0f);
    ImGui::SliderFloat("z", &pScene->p_camera->m_position.z, -10.0f, 10.0f);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VK(result);
}
