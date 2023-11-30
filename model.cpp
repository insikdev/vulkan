#include "pch.h"
#include "model.h"
#include "vk_device.h"
#include "vk_buffer.h"

Model::Model(const Device* pDevice, const MeshData& data)
    : p_device { pDevice }
{
    CreateVertexBuffer(data.vertices);
    CreateIndexBuffer(data.indices);
    CreateUniformbuffer();
}

Model::~Model()
{
    delete m_vertexBuffer;
    delete m_index;
    m_uniform->UnmapMemory();
    delete m_uniform;
}

void Model::Bind(VkCommandBuffer commandBuffer) const
{
    VkBuffer buffers[] = { m_vertexBuffer->GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_index->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Model::Draw(VkCommandBuffer commandBuffer) const
{
    vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
}

Mat4 Model::GetWorldMatrix() const
{
    return m_transform.GetWorldMatrix();
}

void Model::Update(float dt)
{
    // m_transform.RotateX(dt);
    // m_transform.RotateY(dt);
    m_transform.RotateZ(dt);
    UpdateUniformBuffer();
}

void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBufferCreateInfo stagingBufferCreateInfo {};
    {
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.size = bufferSize;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkMemoryPropertyFlags stagingBufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer { p_device, stagingBufferCreateInfo, stagingBufferMemoryPropertyFlags };

    stagingBuffer.MapMemory();
    memcpy(stagingBuffer.GetMappedPtr(), vertices.data(), static_cast<size_t>(bufferSize));
    stagingBuffer.UnmapMemory();

    VkBufferCreateInfo vertexBufferCreateInfo {};
    {
        vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferCreateInfo.size = bufferSize;
        vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkMemoryPropertyFlags vertexBufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_vertexBuffer = new Buffer { p_device, vertexBufferCreateInfo, vertexBufferMemoryPropertyFlags };
    m_vertexBuffer->Copy(stagingBuffer.GetBuffer());
}

void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    m_indexCount = static_cast<uint32_t>(indices.size());
    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

    VkBufferCreateInfo stagingBufferCreateInfo {};
    {
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.size = bufferSize;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VkMemoryPropertyFlags stagingBufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer { p_device, stagingBufferCreateInfo, stagingBufferMemoryPropertyFlags };

    stagingBuffer.MapMemory();
    memcpy(stagingBuffer.GetMappedPtr(), indices.data(), static_cast<size_t>(bufferSize));
    stagingBuffer.UnmapMemory();

    VkBufferCreateInfo indexBufferCreateInfo {};
    {
        indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferCreateInfo.size = bufferSize;
        indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkMemoryPropertyFlags indexBufferMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_index = new Buffer { p_device, indexBufferCreateInfo, indexBufferMemoryPropertyFlags };
    m_index->Copy(stagingBuffer.GetBuffer());
}

void Model::CreateUniformbuffer()
{
    VkBufferCreateInfo createInfo {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = sizeof(ModelUniform);
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    m_uniform = new Buffer { p_device, createInfo, memoryPropertyFlags };
    m_uniform->MapMemory();
}

void Model::UpdateUniformBuffer()
{
    ModelUniform modelUniformData {};
    modelUniformData.world = m_transform.GetWorldMatrix();

    m_uniform->InvalidateMappedMemory();
    memcpy(m_uniform->GetMappedPtr(), &modelUniformData, sizeof(ModelUniform));
    m_uniform->FlushMappedMemory();
}
