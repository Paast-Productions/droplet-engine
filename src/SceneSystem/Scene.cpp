#include "Scene.hpp"

void Scene::Update(float p_deltaTime)
{
    if (!m_loaded || !m_active)
    {
        return;
    }

    m_root->Update(p_deltaTime);
}