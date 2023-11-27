#include "pch.h"
#include "model.h"
#include "device.h"

Model::Model(const Device* pDevice, const MeshData& data)
    : p_device { pDevice }
{
    CreateVertexBuffer(data.vertices);
    CreateIndexBuffer(data.indices);
}

Model::~Model()
{
    vkDestroyBuffer(p_device->GetDevice(), m_vertexBuffer, nullptr);
    vkFreeMemory(p_device->GetDevice(), m_vertexBufferMemory, nullptr);
    vkDestroyBuffer(p_device->GetDevice(), m_indexBuffer, nullptr);
    vkFreeMemory(p_device->GetDevice(), m_indexBufferMemory, nullptr);
}

void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    p_device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(p_device->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(p_device->GetDevice(), stagingBufferMemory);

    p_device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
    p_device->CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(p_device->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(p_device->GetDevice(), stagingBufferMemory, nullptr);
}

void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    m_indexCount = static_cast<uint32_t>(indices.size());
    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    p_device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(p_device->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(p_device->GetDevice(), stagingBufferMemory);

    p_device->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);
    p_device->CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(p_device->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(p_device->GetDevice(), stagingBufferMemory, nullptr);
}
