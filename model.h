#pragma once

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
    inline uint32_t GetVertexCount() const { return m_vertexCount; }
    inline VkBuffer GetIndexBuffer() const { return m_indexBuffer; }
    inline uint32_t GetIndexCount() const { return m_indexCount; }

private:
    void CreateVertexBuffer(const std::vector<Vertex>&);
    void CreateIndexBuffer(const std::vector<uint32_t>&);

private:
    const Device* p_device;

private:
    uint32_t m_indexCount;
    uint32_t m_vertexCount;
    VkBuffer m_indexBuffer;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    VkDeviceMemory m_vertexBufferMemory;
};
