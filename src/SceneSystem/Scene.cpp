#include "Scene.hpp"
#include "Node.hpp"

Scene::Scene(const std::string& p_name)
    : m_name(p_name), 
    m_root(std::make_shared<Node>("Root"))
{
}

void Scene::Update(float p_deltaTime)
{
    if (!m_loaded || !m_active)
    {
        return;
    }

    m_root->Update(p_deltaTime);
}

void Scene::Load()
{
    if (m_loaded)
    {
        return;
    }

    m_loaded = true;
}

void Scene::Unload()
{
    if (!m_loaded)
    {
        return;
    }

    m_active = false;
    m_loaded = false;
}

void Scene::Render()
{
    if (!m_loaded || !m_active)
    {
        return;
    }

    m_root->Render();
}

std::shared_ptr<Node> Scene::GetRoot() const
{
    return m_root;
}

const std::string& Scene::GetName() const
{
    return m_name;
}

bool Scene::IsLoaded() const
{
    return m_loaded;
}

bool Scene::IsActive() const
{
    return m_active;
}

void Scene::SetActive(bool p_active)
{
    m_active = p_active;
}