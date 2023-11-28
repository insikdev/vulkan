#include "pch.h"
#include "scene.h"
#include "model.h"
#include "camera.h"

Scene::Scene(Camera* pCamera)
    : p_camera { pCamera }
{
}

Scene::~Scene()
{
    delete p_camera;

    for (const Model* m : m_models) {
        delete m;
    }
}

void Scene::AddModel(const Model* model)
{
    m_models.push_back(model);
}

CameraUniform Scene::GetViewProjMatrix(void) const
{
    CameraUniform out {};
    out.view = p_camera->GetViewMatrix();
    out.proj = p_camera->GetProjectionMatrix();
    return out;
}
