#pragma once

#include "transform.h"
class Device;
class Buffer;

class Model {
public:
    Model(const Device*, const MeshData&);
    ~Model();
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

public: // getter
    uint32_t GetIndexCount() const { return m_indexCount; }

public:
    void Update(float dt);
    void Bind(VkCommandBuffer) const;
    void Draw(VkCommandBuffer) const;
    Mat4 GetWorldMatrix() const;
    void SetDescriptorSet(VkDescriptorSet, VkImageView, VkSampler);

private:
    void CreateVertexBuffer(const std::vector<Vertex>&);
    void CreateIndexBuffer(const std::vector<uint32_t>&);
    void CreateUniformbuffer();
    void UpdateUniformBuffer();

private:
    const Device* p_device;

public:
    Transform m_transform;
    uint32_t m_indexCount;
    Buffer* m_vertexBuffer;
    Buffer* m_index;
    Buffer* m_uniform;
    VkDescriptorSet m_descriptorSet;
};
