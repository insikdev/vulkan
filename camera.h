#pragma once

class Camera {
public:
    Camera(float aspectRatio)
        : m_aspect { aspectRatio }
    {
    }

    ~Camera() = default;

public:
    Mat4 GetViewMatrix() { return glm::lookAt(m_position, m_target, m_up); }
    Mat4 GetProjectionMatrix() { return glm::perspective(m_fov, m_aspect, m_near, m_far); }

public:
    Vec3 m_position { Vec3 { 0.0f, 0.0f, 5.0f } };
    Vec3 m_target { Vec3 { 0.0f, 0.0f, 0.0f } };
    Vec3 m_up { Vec3 { 0.0f, -1.0f, 0.0f } };

public:
    float m_fov { glm::radians(70.0f) };
    float m_aspect;
    float m_near { 0.1f };
    float m_far { 100.0f };
};
