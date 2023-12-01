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

Renderer::Renderer(Device* pDevice, SwapChain* pSwapChain, const Pipeline* pPipeline)
    : p_device { pDevice }
    , p_swapChain { pSwapChain }
    , p_pipeline { pPipeline }
{
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
    p_commandPool = new CommandPool { p_device };
    m_commandBuffer = p_commandPool->AllocateCommandBuffer().GetHandle();

    std::vector<VkDescriptorPoolSize> poolSizes {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
    };

    p_descriptorPool = new DescriptorPool { p_device, poolSizes };
    CreateSyncObjects();
}

Renderer::~Renderer()
{
    vkDestroySemaphore(p_device->GetDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(p_device->GetDevice(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(p_device->GetDevice(), inFlightFence, nullptr);

    delete p_commandPool;
    delete p_descriptorPool;
}

void Renderer::SetScene(Scene* pScene)
{
    p_scene = pScene;

    for (auto& model : p_scene->GetModels()) {
        VkDescriptorSet dstSet = p_descriptorPool->AllocateDescriptorSet(p_pipeline->GetDescriptorSetLayouts());
        model->SetDescriptorSet(dstSet);
    }
}

void Renderer::Update(float dt)
{
    for (auto& model : p_scene->GetModels()) {
        model->Update(dt);
    }
}

void Renderer::Render()
{
    vkWaitForFences(p_device->GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(p_device->GetDevice(), p_swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    if (result != VK_SUBOPTIMAL_KHR) {
        CHECK_VK(result);
    }

    vkResetFences(p_device->GetDevice(), 1, &inFlightFence);

    VkCommandBuffer commandBuffer = m_commandBuffer;

    RecordCommandBuffer(commandBuffer, imageIndex);

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

    result = vkQueuePresentKHR(p_device->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    CHECK_VK(result);
}

void Renderer::UpdateSwapChain(SwapChain* pSwapChain)
{
    p_swapChain = pSwapChain;
    p_swapChain->CreateFrameBuffer(p_pipeline->GetRenderPass());
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

    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    CHECK_VK(result);
    result = vkCreateSemaphore(p_device->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
    CHECK_VK(result);
    result = vkCreateFence(p_device->GetDevice(), &fenceInfo, nullptr, &inFlightFence);
    CHECK_VK(result);
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

    auto matrix = p_scene->GetViewProjMatrix();
    vkCmdPushConstants(commandBuffer, p_pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CameraUniform), &matrix);

    for (int i = 0; i < p_scene->GetModels().size(); i++) {
        Model* model = p_scene->GetModels()[i];
        model->Bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline->GetPipelineLayout(), 0, 1, &model->m_descriptorSet, 0, nullptr);
        model->Draw(commandBuffer);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera");

    ImGui::SliderFloat("x", &p_scene->p_camera->m_position.x, -10.0f, 10.0f);
    ImGui::SliderFloat("y", &p_scene->p_camera->m_position.y, -10.0f, 10.0f);
    ImGui::SliderFloat("z", &p_scene->p_camera->m_position.z, -10.0f, 10.0f);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VK(result);
}
