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
    CameraUniform GetViewProjMatrix(void) const;

public:
    Camera* p_camera;

private:
    std::vector<Model*> m_models;
};
