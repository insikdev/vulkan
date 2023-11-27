#include "pch.h"
#include "app.h"
#include "window.h"
#include "instance.h"
#include "surface.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"
#include "renderer.h"
#include "scene.h"
#include "model.h"
#include "extension.h"
#include "shader.h"
#include <sstream>
#include "geometry_helper.h"

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

    p_scene = new Scene {};

    p_scene->AddModel(new Model(p_device, Geometry::CreateTriangle()));
    p_scene->AddModel(new Model(p_device, Geometry::CreateRectangle()));
}

App::~App()
{
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
            p_renderer->DrawFrame(p_scene);
            CalculateFrameRate();
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

void App::CalculateFrameRate()
{
    currentTime = glfwGetTime();
    double delta = currentTime - lastTime;

    if (delta >= 1) {
        int framerate { std::max(1, int(numFrames / delta)) };
        std::stringstream title;
        title << "Running at " << framerate << " fps.";
        glfwSetWindowTitle(p_window->GetWindow(), title.str().c_str());
        lastTime = currentTime;
        numFrames = -1;
        frameTime = float(1000.0 / framerate);
    }

    ++numFrames;
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
