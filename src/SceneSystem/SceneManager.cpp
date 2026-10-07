#include "SceneManager.hpp"
#include "SceneFactory.hpp"
#include "SceneSerializer.hpp"

#include <Core/IoManager.hpp>
#include "Scene.hpp"
#include <algorithm>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;

void SceneManager::LoadScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    if (m_scenes.contains(p_name))
    {
        throw std::runtime_error("Cannot load Scene '" + p_name + "': Scene already exists.");
    }

    std::shared_ptr<Scene> scene = SceneFactory::CreateScene(p_name);

    if (!scene)
    {
        throw std::runtime_error("Cannot load Scene '" + p_name + "': SceneFactory failed to create the Scene.");
    }

    scene->Load();

    m_scenes[p_name] = scene;
}

void SceneManager::UnloadScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        throw std::runtime_error("Cannot unload Scene '" + p_name + "': Scene does not exist.");
    }

    if (it->second->IsActive())
    {
        DeactivateScene(p_name);
    }

    it->second->Unload();

    m_scenes.erase(it);
}

void SceneManager::ActivateScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto scene = GetScene(p_name);

    if (!scene)
    {
        throw std::runtime_error("Cannot activate Scene '" + p_name + "': Scene does not exist.");
    }

    if (!scene->IsLoaded())
    {
        throw std::runtime_error("Cannot activate Scene '" + p_name + "': Scene is not loaded.");
    }

    if (scene->IsActive())
    {
        throw std::runtime_error("Cannot activate Scene '" + p_name + "': Scene is already active.");
    }

    scene->SetActive(true);

    m_activeScenes.push_back(scene);
}

void SceneManager::DeactivateScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto scene = GetScene(p_name);

    if (!scene)
    {
        throw std::runtime_error("Cannot deactivate Scene '" + p_name + "': Scene does not exist.");
    }

    if (!scene->IsLoaded())
    {
        throw std::runtime_error("Cannot deactivate Scene '" + p_name + "': Scene is not loaded.");
    }

    if (!scene->IsActive())
    {
        throw std::runtime_error("Cannot deactivate Scene '" + p_name + "': Scene is already inactive.");
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
    ZoneScoped;

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
    ZoneScoped;

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

void Droplet::Scene::SceneManager::SerializeToFile(std::shared_ptr<Scene> p_scene,  const std::string &p_filePath)
{
    SceneSerializer serializer;
    nlohmann::json json;

    json = serializer.SerializeScene(p_scene);
    Droplet::Core::JsonIO::Write(p_filePath, json);
}

void Droplet::Scene::SceneManager::LoadFromFile(const std::string &p_filePath)
{
    SceneSerializer serializer;
    nlohmann::json json = Droplet::Core::JsonIO::Read(p_filePath);

    if (!json.contains("name"))
    {
        throw std::runtime_error("Invalid json was sent to LoadFromFile");
    }

    if (m_scenes.contains(json["name"]))
    {
        throw std::runtime_error("Cannot load Scene " + json["name"] );
    }

    std::shared_ptr<Scene> scene = SceneFactory::CreateScene(json["name"]);

    if (!scene)
    {
        throw std::runtime_error("Cannot load Scene "+ json["name"]);
    }

    serializer.DeserializeScene(json, scene);
    scene->Load();

    m_scenes[json["name"]] = scene;

}
