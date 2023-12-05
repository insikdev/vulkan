#pragma once

class Transform {
public:
    Transform() = default;
    ~Transform() = default;

public:
    Mat4 GetWorldMatrix() const
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, m_position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, m_scale);
        return modelMatrix;
    }

    void RotateZ(float dt)
    {
        m_rotation.z += rotation_speed * dt;
    }
    void RotateY(float dt)
    {
        m_rotation.y += rotation_speed * dt;
    }
    void RotateX(float dt)
    {
        m_rotation.x += rotation_speed * dt;
    }

public:
    const float rotation_speed = 100.0f;
    Vec3 m_scale { Vec3 { 1.0f } };
    Vec3 m_rotation { Vec3 { 0.0f } };
    Vec3 m_position { Vec3 { 0.0f } };
};
