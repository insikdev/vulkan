#pragma once

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "vulkan-1.lib")

#include <cassert>
#include <vector>
#include <iostream>
#include <array>
#include <set>
#include <algorithm>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CHECK_VK(RESULT) assert(RESULT == VK_SUCCESS)

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;

struct Vertex {
    Vec3 pos;
    Vec3 color;
    Vec2 texcoord;

    static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> desc(1);

        desc[0].binding = 0;
        desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        desc[0].stride = sizeof(Vertex);

        return desc;
    }
    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> descs(3);

        descs[0].binding = 0;
        descs[0].location = 0;
        descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        descs[0].offset = offsetof(Vertex, pos);

        descs[1].binding = 0;
        descs[1].location = 1;
        descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descs[1].offset = offsetof(Vertex, color);

        descs[2].binding = 0;
        descs[2].location = 2;
        descs[2].format = VK_FORMAT_R32G32_SFLOAT;
        descs[2].offset = offsetof(Vertex, texcoord);

        return descs;
    }
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct CameraUniform {
    alignas(16) Mat4 view;
    alignas(16) Mat4 proj;
};

struct ModelUniform {
    alignas(16) Mat4 world;
};

struct PhongModel : ModelUniform {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;
};

struct LightUniform {
    Vec3 pos;
    Vec3 dir;
    Vec3 color;
};