#include "pch.h"
#include "renderer.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"
#include "scene.h"
#include "descriptor_helper.h"
#include "model.h"

Renderer::Renderer(const Device* pDevice, const SwapChain* pSwapChain, const Pipeline* pPipeline)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
    , p_pipeline { pPipeline }
{
    createUniformBuffers();
    CreateFrameBuffers();
    CreateSyncObjects();
    CreateCommandBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

Renderer::~Renderer()
{
    delete p_descriptor;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(p_device->GetDevice(), uniformBuffers[i], nullptr);
        vkFreeMemory(p_device->GetDevice(), uniformBuffersMemory[i], nullptr);
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(p_device->GetDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(p_device->GetDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(p_device->GetDevice(), inFlightFences[i], nullptr);
    }

    for (auto framebuffer : m_frameBuffers) {
        vkDestroyFramebuffer(p_device->GetDevice(), framebuffer, nullptr);
    }
}

void Renderer::DrawFrame(const Scene* pScene)
{
    static float time = 0.0f;

    vkWaitForFences(p_device->GetDevice(), 1, &inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    UniformBufferObject ubo {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), p_swapChain->GetExtent2D().width / (float)p_swapChain->GetExtent2D().height, 0.1f, 10.0f);
    // ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[m_currentFrame], &ubo, sizeof(ubo));

    time += 0.1f;

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    if (result != VK_SUBOPTIMAL_KHR) {
        CHECK_VK(result);
    }

    vkResetFences(p_device->GetDevice(), 1, &inFlightFences[m_currentFrame]);

    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    vkResetCommandBuffer(commandBuffer, 0);
    RecordCommandBuffer(commandBuffer, imageIndex, pScene);

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[m_currentFrame] };
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
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[m_currentFrame];
    }

    result = vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, inFlightFences[m_currentFrame]);
    CHECK_VK(result);

    VkSwapchainKHR swapChains[] = { p_swapChain->GetSwapChain() };
    VkPresentInfoKHR presentInfo {};
    {
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[m_currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
    }

    result = vkQueuePresentKHR(p_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    CHECK_VK(result);

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::UpdateSwapChain(const SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;

    for (auto framebuffer : m_frameBuffers) {
        vkDestroyFramebuffer(p_device->GetDevice(), framebuffer, nullptr);
    }

    CreateFrameBuffers();
}

void Renderer::createDescriptorSets()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto descriptorSet = p_descriptor->AllocateDescriptorSet(p_pipeline->GetDescriptorSetLayouts());
        descriptorSets.push_back(descriptorSet);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo {};
        {
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
        }

        VkWriteDescriptorSet descriptorWrite {};
        {
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional
        }

        p_descriptor->UpdateDescriptorSet(descriptorWrite);
    }
}

void Renderer::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    p_descriptor = new DescriptorHelper { p_device, poolSize, MAX_FRAMES_IN_FLIGHT };
}

void Renderer::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        p_device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(p_device->GetDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void Renderer::CreateFrameBuffers()
{
    m_frameBuffers.resize(p_swapChain->GetImageViews().size());

    for (size_t i = 0; i < m_frameBuffers.size(); i++) {
        VkImageView attachments[] = { p_swapChain->GetImageViews()[i] };

        VkFramebufferCreateInfo framebufferInfo {};
        {
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = p_pipeline->GetRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = p_swapChain->GetExtent2D().width;
            framebufferInfo.height = p_swapChain->GetExtent2D().height;
            framebufferInfo.layers = 1;
        }

        VkResult result = vkCreateFramebuffer(p_device->GetDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]);
        CHECK_VK(result);
    }
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

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        CHECK_VK(result);
        result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        CHECK_VK(result);
        result = vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]);
        CHECK_VK(result);
    }
}

void Renderer::CreateCommandBuffers()
{
    p_device->CreateCommandBuffers(m_commandBuffers);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene* pScene)
{
    VkCommandBufferBeginInfo beginInfo {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    }

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    CHECK_VK(result);

    VkRenderPassBeginInfo renderPassInfo {};
    {
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = p_pipeline->GetRenderPass();
        renderPassInfo.framebuffer = m_frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = p_swapChain->GetExtent2D();
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &m_clearColor;
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

    for (const auto& model : pScene->GetModels()) {
        VkBuffer buffers[] = { model->GetVertexBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, model->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipelineLayout(), 0, 1, &descriptorSets[m_currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, model->GetIndexCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VK(result);
}
