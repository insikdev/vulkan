#pragma once

class Model;
class Camera;

class Scene {
public:
    Scene(Camera*);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

public:
    void AddModel(Model*);
    std::vector<Model*> GetModels() const { return m_models; }

public:
    Camera* p_camera;
    Vec3 lightPos { Vec3 { 0, 0, 3.0f } };
    Vec3 lightDir { Vec3 { 1.0f } };
    Vec3 lightColor { Vec3 { 1.0f } };

private:
    std::vector<Model*> m_models;
};
