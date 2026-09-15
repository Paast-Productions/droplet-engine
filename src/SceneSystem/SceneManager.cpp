#include "SceneManager.hpp"
#include "SceneFactory.hpp"
#include "Scene.hpp"

bool SceneManager::LoadScene(const std::string& p_name)
{
    if (m_scenes.contains(p_name))
    {
        return true;
    }

    auto scene = SceneFactory::CreateScene(p_name);

    if (!scene)
    {
        return false;
    }

    scene->Load();

    m_scenes.emplace(p_name, scene);

    return true;
}  