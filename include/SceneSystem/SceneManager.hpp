#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>   

class Scene;

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    bool LoadScene(const std::string& p_name);
    void UnloadScene(const std::string& p_name);

    bool ActivateScene(const std::string& p_name);
    bool DeactivateScene(const std::string& p_name);

    std::shared_ptr<Scene> GetScene(const std::string& p_name) const;

    const std::vector<std::weak_ptr<Scene>>& GetActiveScenes() const;

    void Update(float p_deltaTime);
    void Render();

private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::vector<std::weak_ptr<Scene>> m_activeScenes;
};