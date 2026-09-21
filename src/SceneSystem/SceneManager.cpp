#include "SceneManager.hpp"
#include "SceneFactory.hpp"
#include "Scene.hpp"
#include <algorithm>

bool SceneManager::LoadScene(const std::string &p_name)
{
    if (m_scenes.contains(p_name))
    {
        return false;
    }

    std::shared_ptr<Scene> scene = SceneFactory::CreateScene(p_name);

    if (!scene)
    {
        return false;
    }

    scene->Load();

    m_scenes[p_name] = scene;

    return true;
}

void SceneManager::UnloadScene(const std::string &p_name)
{
    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        return;
    }

    DeactivateScene(p_name);

    it->second->Unload();

    m_scenes.erase(it);
}

bool SceneManager::ActivateScene(const std::string &p_name)
{
    auto scene = GetScene(p_name);

    if (!scene)
    {
        return false;
    }

    if (!scene->IsLoaded())
    {
        return false;
    }

    if (scene->IsActive())
    {
        return true;
    }

    scene->SetActive(true);

    m_activeScenes.push_back(scene);

    return true;
}

bool SceneManager::DeactivateScene(const std::string &p_name)
{
    auto scene = GetScene(p_name);

    if (!scene)
    {
        return false;
    }

    scene->SetActive(false);

    m_activeScenes.erase(std::remove_if(m_activeScenes.begin(), m_activeScenes.end(), [&p_name](const std::weak_ptr<Scene> &p_scene)
    {
        auto scene = p_scene.lock();

        if (!scene)
        {
            return true;
        }

        if (scene->GetName() == p_name)
        {
            return true;
        }

        return false;
    }
    ),
    m_activeScenes.end());

    return true;
}

std::shared_ptr<Scene> SceneManager::GetScene(const std::string &p_name) const
{
    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        return nullptr;
    }

    return it->second;
}

const std::vector<std::weak_ptr<Scene>> &SceneManager::GetActiveScenes() const
{
    return m_activeScenes;
}

void SceneManager::Update(float p_deltaTime)
{
    for (auto it = m_activeScenes.begin(); it != m_activeScenes.end();)
    {
        auto scene = it->lock();

        if (!scene)
        {
            it = m_activeScenes.erase(it);
            continue;
        }

        scene->Update(p_deltaTime);

        ++it;
    }
}

void SceneManager::Render()
{
    for (auto it = m_activeScenes.begin(); it != m_activeScenes.end();)
    {
        auto scene = it->lock();

        if (!scene)
        {
            it = m_activeScenes.erase(it);
            continue;
        }

        scene->Render();

        ++it;
    }
}