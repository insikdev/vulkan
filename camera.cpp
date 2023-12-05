#include "pch.h"
#include "camera.h"
#include "vk_device.h"
#include "vk_buffer.h"

Camera::Camera(Device* pDevice, float aspectRatio)
    : p_device { pDevice }
    , m_aspect { aspectRatio }
{
    CreateUniform();
}

Camera::~Camera()
{
    m_uniform->UnmapMemory();
    delete m_uniform;
}

void Camera::CreateUniform()
{
    VkBufferCreateInfo createInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    {
        createInfo.size = sizeof(CameraUniform);
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    m_uniform = new Buffer { p_device, createInfo, memoryPropertyFlags };
    m_uniform->MapMemory();
}

void Camera::UpdateUniform()
{
    CameraUniform uniformData {};
    uniformData.view = glm::lookAt(m_position, m_target, m_up);
    uniformData.proj = glm::perspective(m_fov, m_aspect, m_near, m_far);

    m_uniform->InvalidateMappedMemory();
    memcpy(m_uniform->GetMappedPtr(), &uniformData, sizeof(CameraUniform));
    m_uniform->FlushMappedMemory();
}

void Camera::SetDescriptorSet(VkDescriptorSet dstSet)
{
    m_descriptorSet = dstSet;
    VkDescriptorBufferInfo bufferInfo {};
    {
        bufferInfo.buffer = m_uniform->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraUniform);
    }

    VkWriteDescriptorSet uniformDS {};
    {
        uniformDS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformDS.dstSet = dstSet;
        uniformDS.dstBinding = 0;
        uniformDS.dstArrayElement = 0;
        uniformDS.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformDS.descriptorCount = 1;
        uniformDS.pBufferInfo = &bufferInfo;
    }

    vkUpdateDescriptorSets(p_device->GetDevice(), 1, &uniformDS, 0, nullptr);
}
