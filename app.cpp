#include "pch.h"
#include "app.h"
#include "vk_window.h"
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swap_chain.h"
#include "vk_pipeline.h"
#include "renderer.h"
#include "scene.h"
#include "model.h"
#include "extension.h"
#include "shader.h"
#include "camera.h"
#include <sstream>
#include "geometry_helper.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "vk_command_pool.h"
#include "vk_command_buffer.h"
#include "vk_descriptor_pool.h"

App::App()
{
    p_window = new Window { "vulkan_tutorial", this };
    p_instance = new Instance { m_requiredLayers, m_requiredLayerExtensions };
    p_surface = new Surface { p_window, p_instance };
    p_device = new Device { p_instance, p_surface, m_requiredDeviceExtensions };
    p_swapChain = new SwapChain { p_window, p_surface, p_device };
    p_pipeline = new Pipeline { p_device, p_swapChain };
    p_renderer = new Renderer { p_device, p_swapChain, p_pipeline };

    SetupDebugMessenger();

    float aspectRatio = p_swapChain->GetExtent2D().width / static_cast<float>(p_swapChain->GetExtent2D().height);

    p_scene = new Scene { new Camera { aspectRatio } };

    auto cube1 = new Model(p_device, Geometry::CreateCube());
    p_scene->AddModel(cube1);

    // auto cube2 = new Model(p_device, Geometry::CreateCube());
    // cube2->m_transform.m_position.x = -1.0f;
    // cube2->m_transform.m_position.y = -2.0f;
    // cube2->m_transform.m_position.z = -3.0f;
    // p_scene->AddModel(cube2);
    // auto cube3 = new Model(p_device, Geometry::CreateCube());
    // cube3->m_transform.m_position.x = 1.0f;
    // cube3->m_transform.m_position.y = 2.0f;
    // cube3->m_transform.m_position.z = 3.0f;
    // p_scene->AddModel(cube3);

    p_renderer->SetScene(p_scene);
    InitGui();
}

App::~App()
{
    delete p_descriptor;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Extension::DestroyDebugUtilsMessengerEXT(p_instance->GetInstance(), m_debugMessenger, nullptr);

    delete p_scene;
    delete p_renderer;
    delete p_pipeline;
    delete p_swapChain;
    delete p_device;
    delete p_surface;
    delete p_instance;
    delete p_window;
}

void App::Run()
{
    while (!p_window->ShouldClose()) {
        if (m_resized) {
            HandleResize();
        } else {
            glfwPollEvents();
            p_renderer->Update(ImGui::GetIO().DeltaTime);
            p_renderer->Render();
        }
    }

    vkDeviceWaitIdle(p_device->GetDevice());
}

void App::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Extension::populateDebugMessengerCreateInfo(createInfo);

    VkResult result = Extension::CreateDebugUtilsMessengerEXT(p_instance->GetInstance(), &createInfo, nullptr, &m_debugMessenger);
    CHECK_VK(result);
}

void App::HandleResize()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(p_window->GetWindow(), &width, &height);

    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(p_window->GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(p_device->GetDevice());

    p_window->SetWidth(width);
    p_window->SetHeight(height);

    delete p_swapChain;
    p_swapChain = new SwapChain { p_window, p_surface, p_device };
    p_pipeline->UpdateSwapChain(p_swapChain);
    p_renderer->UpdateSwapChain(p_swapChain);

    m_resized = false;
}

void App::InitGui()
{
    CommandPool* p_command = new CommandPool { p_device };
    CommandBuffer commandBuffer = p_command->AllocateCommandBuffer();

    std::vector<VkDescriptorPoolSize> pool_sizes = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };
    p_descriptor = new DescriptorPool { p_device, pool_sizes, 1 };

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(p_window->GetWindow(), true);

    ImGui_ImplVulkan_InitInfo info {};
    {
        info.Instance = p_instance->GetInstance();
        info.PhysicalDevice = p_device->GetPhysicalDevice();
        info.Device = p_device->GetDevice();
        info.Queue = p_device->GetQueue();
        info.DescriptorPool = p_descriptor->GetPool();
        info.MinImageCount = p_swapChain->GetImageCount() - 1;
        info.ImageCount = p_swapChain->GetImageCount();
    }

    ImGui_ImplVulkan_Init(&info, p_pipeline->GetRenderPass());

    commandBuffer.Begin();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer.GetHandle());
    commandBuffer.End();

    VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    {
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer.GetPtr();
    }

    CHECK_VK(vkQueueSubmit(p_device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    CHECK_VK(vkQueueWaitIdle(p_device->GetQueue()));

    vkDeviceWaitIdle(p_device->GetDevice());
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    delete p_command;
}
