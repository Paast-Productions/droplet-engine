#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Scene;

/// @brief Manages the Scenes used by the application.
///
/// SceneManager is responsible for loading, unloading, activating,
/// and deactivating Scenes. It also updates and renders all
/// currently active Scenes.
///
/// Loaded Scenes are stored using shared ownership, while the list
/// of active Scenes uses weak references to avoid additional ownership.
class SceneManager
{
public:

    /// @brief Constructs an empty SceneManager.
    SceneManager() = default;

    /// @brief Destroys the SceneManager.
    ~SceneManager() = default;

    /// @brief Loads a Scene with the specified name.
    ///
    /// The Scene is created using the SceneFactory and stored in
    /// the SceneManager.
    ///
    /// @param p_name Name of the Scene to load.
    ///
    /// @return true if the Scene was successfully loaded,
    /// otherwise false.
    bool LoadScene(const std::string &p_name);

    /// @brief Unloads a Scene with the specified name.
    ///
    /// The Scene is deactivated if necessary, unloaded, and removed
    /// from the SceneManager.
    ///
    /// @param p_name Name of the Scene to unload.
    void UnloadScene(const std::string &p_name);

    /// @brief Activates a loaded Scene.
    ///
    /// An active Scene is updated and rendered by the SceneManager.
    ///
    /// @param p_name Name of the Scene to activate.
    ///
    /// @return true if the Scene was successfully activated,
    /// otherwise false.
    bool ActivateScene(const std::string &p_name);

    /// @brief Deactivates an active Scene.
    ///
    /// A deactivated Scene remains loaded but is no longer updated
    /// or rendered.
    ///
    /// @param p_name Name of the Scene to deactivate.
    ///
    /// @return true if the Scene was successfully deactivated,
    /// otherwise false.
    bool DeactivateScene(const std::string &p_name);

    /// @brief Gets a loaded Scene by name.
    ///
    /// @param p_name Name of the Scene to retrieve.
    ///
    /// @return A shared pointer to the Scene, or nullptr if the
    /// Scene is not loaded.
    std::shared_ptr<Scene> GetScene(const std::string &p_name) const;

    /// @brief Gets the currently active Scenes.
    ///
    /// The returned list contains weak references to the Scenes that
    /// are currently active. Callers must lock the weak pointers before
    /// accessing the Scenes.
    ///
    /// @return A constant reference to the list of active Scene
    /// weak pointers.
    const std::vector<std::weak_ptr<Scene>> &GetActiveScenes() const;

    /// @brief Updates all active Scenes.
    ///
    /// @param p_deltaTime Time elapsed since the previous update, in seconds.
    void Update(float p_deltaTime);

    /// @brief Renders all active Scenes.
    ///
    /// @note This function is currently a placeholder and does not
    /// perform any rendering.
    void Render();

private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::vector<std::weak_ptr<Scene>> m_activeScenes;
};