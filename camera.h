#pragma once

class Device;
class Buffer;

class Camera {
public:
    Camera(Device*, float aspectRatio);
    ~Camera();

public:
    Mat4 GetViewMatrix() { return glm::lookAt(m_position, m_target, m_up); }
    Mat4 GetProjectionMatrix() { return glm::perspective(m_fov, m_aspect, m_near, m_far); }
    void UpdateUniform();
    void SetDescriptorSet(VkDescriptorSet);

public:
    Vec3 m_position { Vec3 { 0.0f, 0.0f, 5.0f } };
    Vec3 m_target { Vec3 { 0.0f, 0.0f, 0.0f } };
    Vec3 m_up { Vec3 { 0.0f, -1.0f, 0.0f } };
    VkDescriptorSet m_descriptorSet;

private:
    void CreateUniform();

private:
    Device* p_device;
    Buffer* m_uniform;

public:
    float m_fov { glm::radians(70.0f) };
    float m_aspect;
    float m_near { 0.1f };
    float m_far { 100.0f };
};
