#pragma once

#include <memory>
#include <string>

class Node;

/// @brief Represents a scene containing a hierarchy of Nodes.
///
/// A Scene owns a root Node that serves as the entry point to the
/// scene hierarchy. Scenes can be loaded, unloaded, activated,
/// and updated independently.
///
/// Nodes maintain a weak reference to their containing Scene so
/// that they can access Scene-level functionality without creating
/// an ownership cycle.
class Scene: public std::enable_shared_from_this<Scene>
{
public:

    /// @brief Constructs a Scene with the specified name.
    /// @param p_name Name of the Scene.
    explicit Scene(std::string p_name);

    /// @brief Virtual destructor.
    virtual ~Scene() = default;

    /// @brief Loads the Scene.
    ///
    /// Marks the Scene as loaded and assigns the Scene reference to
    /// the root Node. The Scene reference is propagated through the
    /// Node hierarchy.
    ///
    /// Calling Load() on an already loaded Scene has no effect.
    ///
    /// Derived Scenes can override this function to perform additional
    /// loading or initialization.
    virtual void Load();

    /// @brief Unloads the Scene.
    ///
    /// Marks the Scene as unloaded and deactivates it.
    ///
    /// The Node hierarchy remains owned by the Scene and can be loaded
    /// again later.
    ///
    /// Calling Unload() on an already unloaded Scene has no effect.
    ///
    /// Derived Scenes can override this function to perform additional
    /// cleanup.
    virtual void Unload();

    /// @brief Updates the Scene.
    ///
    /// Updates the Scene's Node hierarchy when the Scene is both
    /// loaded and active.
    ///
    /// @param p_deltaTime Time elapsed since the previous update, in seconds.
    virtual void Update(float p_deltaTime);

    /// @brief Renders the Scene.
    ///
    /// @note Currently unused and does not perform any rendering.
    /// Rendering functionality may be moved to a separate rendering
    /// system in the future.
    virtual void Render();

    /// @brief Gets the root Node of the Scene.
    ///
    /// The root Node serves as the starting point of the Scene's
    /// Node hierarchy.
    ///
    /// @return Shared pointer to the root Node.
    std::shared_ptr<Node> GetRoot() const;

    /// @brief Gets the Scene's name.
    ///
    /// @return A constant reference to the Scene's name.
    const std::string& GetName() const;

    /// @brief Checks whether the Scene is loaded.
    ///
    /// @return true if the Scene is loaded, otherwise false.
    bool IsLoaded() const;

    /// @brief Checks whether the Scene is active.
    ///
    /// An inactive Scene does not perform its normal update or
    /// rendering operations.
    ///
    /// @return true if the Scene is active, otherwise false.
    bool IsActive() const;

    /// @brief Sets whether the Scene is active.
    ///
    /// @param p_active true to activate the Scene, false to deactivate it.
    void SetActive(bool p_active);

private:
    std::string m_name;
    std::shared_ptr<Node> m_root;

    bool m_loaded = false;
    bool m_active = false;
};     