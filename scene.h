#pragma once

class Model;

class Scene {
public:
    Scene() = default;
    ~Scene();
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

public:
    void AddModel(const Model*);
    std::vector<const Model*> GetModels() const { return m_models; }

private:
    std::vector<const Model*> m_models;
};
