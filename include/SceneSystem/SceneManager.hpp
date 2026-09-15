#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Scene;

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    bool LoadScene(const std::string& p_name);
    void UnloadScene(const std::string& p_name);

    bool SetActiveScene(const std::string& p_name);

    std::shared_ptr<Scene> GetScene(const std::string& p_name) const;

    std::shared_ptr<Scene> GetActiveScene() const;

    void Update(float p_deltaTime);
    void Render();

private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;

    std::shared_ptr<Scene> m_activeScene;
};