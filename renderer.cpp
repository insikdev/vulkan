#include "pch.h"
#include "renderer.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"
#include "scene.h"
#include "model.h"

Renderer::Renderer(const Device* pDevice, const SwapChain* pSwapChain, const Pipeline* pPipeline)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
    , p_pipeline { pPipeline }
{
    CreateFrameBuffers();
    CreateSyncObjects();
}

Renderer::~Renderer()
{
    vkDestroySemaphore(p_device->GetDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(p_device->GetDevice(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(p_device->GetDevice(), inFlightFence, nullptr);

    for (auto framebuffer : m_frameBuffers) {
        vkDestroyFramebuffer(p_device->GetDevice(), framebuffer, nullptr);
    }
}

void Renderer::DrawFrame(const Scene* pScene)
{
    VkResult result;

    vkWaitForFences(p_device->GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(p_device->GetDevice(), 1, &inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBuffer commandBuffer = p_device->GetCommandBuffer();

    vkResetCommandBuffer(commandBuffer, 0);
    RecordCommandBuffer(commandBuffer, imageIndex, pScene);

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
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
    }

    result = vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, inFlightFence);
    CHECK_VK(result);

    VkSwapchainKHR swapChains[] = { p_swapChain->GetSwapChain() };
    VkPresentInfoKHR presentInfo {};
    {
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
    }

    result = vkQueuePresentKHR(p_device->GetQueue(), &presentInfo);
    CHECK_VK(result);
}

void Renderer::CreateFrameBuffers()
{
    m_frameBuffers.resize(p_swapChain->GetImageViews().size());

    for (size_t i = 0; i < m_frameBuffers.size(); i++) {
        VkImageView attachments[] = { p_swapChain->GetImageViews()[i] };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = p_pipeline->GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = p_swapChain->GetExtent2D().width;
        framebufferInfo.height = p_swapChain->GetExtent2D().height;
        framebufferInfo.layers = 1;

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

    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    CHECK_VK(result);
    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
    CHECK_VK(result);

    VkFenceCreateInfo fenceInfo {};
    {
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    result = vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &inFlightFence);
    CHECK_VK(result);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene* pScene)
{
    VkResult result;

    VkCommandBufferBeginInfo beginInfo {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    }

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
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

    for (const auto& model : pScene->GetModels()) {
        VkBuffer buffers[] = { model->GetVertexBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, model->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, model->GetIndexCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VK(result);
}
