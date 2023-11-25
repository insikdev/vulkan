#include "pch.h"
#include "scene.h"
#include "model.h"

Scene::~Scene()
{
    for (const Model* m : m_models) {
        delete m;
    }
}

void Scene::AddModel(const Model* model)
{
    m_models.push_back(model);
}
