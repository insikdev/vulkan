#pragma once

#include "transform.h"
class Device;

class Model {
public:
    Model(const Device*, const MeshData&);
    ~Model();
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

public: // getter
    inline VkBuffer GetVertexBuffer() const { return m_vertexBuffer; }
    inline VkBuffer GetIndexBuffer() const { return m_indexBuffer; }
    inline uint32_t GetIndexCount() const { return m_indexCount; }

public:
    void Update(float dt);
    void Bind(VkCommandBuffer) const;
    void Draw(VkCommandBuffer) const;
    Mat4 GetWorldMatrix() const;

private:
    void CreateVertexBuffer(const std::vector<Vertex>&);
    void CreateIndexBuffer(const std::vector<uint32_t>&);
    void CreateUniformbuffer();
    void UpdateUniformBuffer();

private:
    const Device* p_device;

public:
    uint32_t m_indexCount;
    VkBuffer m_indexBuffer;
    VkBuffer m_vertexBuffer;
    VkBuffer m_uniformBuffer;
    VkDeviceMemory m_indexBufferMemory;
    VkDeviceMemory m_vertexBufferMemory;
    VkDeviceMemory m_uniformBufferMemory;
    Transform m_transform;

private:
};
