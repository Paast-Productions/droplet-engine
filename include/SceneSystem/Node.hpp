#pragma once

#include <utility>
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class Component;
class Scene;

/// @brief Represents a game object in a scene hierarchy.
///
/// A Node can contain child Nodes and Components and has a local
/// transform relative to its parent. Its world transform is calculated
/// from its local transform and the transform of its parent.
///
/// Nodes are managed using shared ownership and maintain a weak
/// reference to their parent to avoid ownership cycles.
class Node : public std::enable_shared_from_this<Node>
{
public:
    /// @brief Constructs a Node with the specified name.
    /// @param p_name Name of the Node.
    explicit Node(std::string p_name);

    /// @brief Virtual destructor.
    virtual ~Node() = default;

    /// @brief Starts the Node, its Components, and its children.
    ///
    /// Calls OnStart() on all Components attached to this Node and then
    /// starts all child Nodes.
    ///
    /// OnStart() should only be called once for each Node activation.
    ///
    /// @note The Node's OnStart() function can be overridden by derived Nodes.
    virtual void OnStart();

    /// @brief Updates the Node, its Components, and its children.
    ///
    /// Updates the Node's transforms, followed by its Components
    /// and child Nodes.
    ///
    /// @param p_deltaTime Time elapsed since the previous update, in seconds.
    virtual void Update(float p_deltaTime);

    /// @brief Renders the Node.
    ///
    /// @note Currently unused and does not perform any rendering.
    /// Rendering functionality may be moved.
    virtual void Render();

    /// @brief Sets whether the Node is active.
    ///
    /// An inactive Node does not update its Components or child Nodes.
    ///
    /// If the Node is activated after its Scene has already started,
    /// OnStart is called automatically.
    ///
    /// @param p_active true to activate the Node, false to deactivate it.
    void SetActive(bool p_active);

    /// @brief Checks whether the Node is active.
    ///
    /// @return true if the Node is active, otherwise false.
    bool IsActive() const;

    // --------------------------------------------------
    // Children
    // --------------------------------------------------

    /// @brief Adds a child Node.
    ///
    /// Sets this Node as the parent of the specified child and adds
    /// the child to the Node hierarchy.
    ///
    /// @param p_child Node to add as a child.
    /// @return The added child Node, or nullptr if p_child is nullptr.
    std::shared_ptr<Node> AddChild(std::shared_ptr<Node> p_child);

    /// @brief Removes a child Node.
    ///
    /// If the specified Node is a child of this Node, it is removed
    /// from the hierarchy and its parent reference is cleared.
    ///
    /// @param p_child Node to remove.
    void RemoveChild(std::shared_ptr<Node> p_child);

    /// @brief Gets the parent Node.
    ///
    /// @return A shared pointer to the parent Node, or nullptr if this
    /// Node has no parent.
    std::shared_ptr<Node> GetParent() const;

    /// @brief Gets the child Nodes.
    ///
    /// @return A constant reference to the vector containing this Node's children.
    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    /// @brief Gets the Scene that owns this Node.
    ///
    /// @return A shared pointer to the Scene, or nullptr if the Node
    /// does not currently belong to a Scene.
    std::shared_ptr<Scene> GetScene() const;

    // --------------------------------------------------
    // Name
    // --------------------------------------------------

    /// @brief Gets the Node's name.
    ///
    /// @return A constant reference to the Node's name.
    const std::string& GetName() const;

    // --------------------------------------------------
    // Components
    // --------------------------------------------------

    /// @brief Adds a Component to the Node.
    ///
    /// Constructs the Component using the provided arguments, assigns
    /// this Node as its owner, stores it, and calls its Initialize() method.
    ///
    /// Multiple Components of the same type can be attached to a Node.
    ///
    /// @tparam T Type of Component to create.
    /// @tparam Args Types of arguments used to construct the Component.
    ///
    /// @param p_args Arguments forwarded to the Component constructor.
    ///
    /// @return A shared pointer to the newly created Component.
    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... p_args)
    {
        auto component =
            std::make_shared<T>(
                std::forward<Args>(p_args)...);

        component->SetOwner(shared_from_this());

        m_components.push_back(component);

        component->Initialize();

        if (m_started)
        {
            component->OnStart();
        }

        return component;
    }

    /// @brief Gets all Components of the specified type.
    ///
    /// Searches the Components attached to this Node and returns all
    /// Components that can be cast to the requested type.
    ///
    /// @tparam T Type of Component to search for.
    ///
    /// @return A vector of shared pointers to the Components, or an empty
    /// vector if no Components of the requested type were found.
    template<typename T>
    std::vector<std::shared_ptr<T>> GetComponents() const
    {
        std::vector<std::shared_ptr<T>> components;

        for (const auto& component : m_components)
        {
            auto result =
                std::dynamic_pointer_cast<T>(component);

            if (result)
            {
                components.push_back(result);
            }
        }

        return components;
    }

    /// @brief Removes the first Component of the specified type.
    ///
    /// Searches the Components attached to this Node and removes the
    /// first Component that matches the requested type.
    ///
    /// @tparam T Type of Component to remove.
    ///
    /// @return true if a Component was removed, otherwise false.
    template<typename T>
    bool RemoveComponent()
    {
        auto it = std::find_if(
            m_components.begin(),
            m_components.end(),
            [](const std::shared_ptr<Component>& p_component)
            {
                return std::dynamic_pointer_cast<T>(p_component)
                    != nullptr;
            });

        if (it == m_components.end())
        {
            return false;
        }

        m_components.erase(it);

        return true;
    }

    // --------------------------------------------------
    // Transforms
    // --------------------------------------------------

    /// @brief Sets the Node's local position.
    ///
    /// @param p_position Local position relative to the parent Node.
    void SetPosition(const glm::vec3& p_position);

    /// @brief Sets the Node's local rotation.
    ///
    /// @param p_rotation Local rotation in radians.
    void SetRotation(const glm::vec3& p_rotation);

    /// @brief Sets the Node's local scale.
    ///
    /// @param p_scale Local scale relative to the parent Node.
    void SetScale(const glm::vec3& p_scale);

    /// @brief Gets the Node's local position.
    ///
    /// @return A constant reference to the local position.
    const glm::vec3& GetPosition() const;

    /// @brief Gets the Node's local rotation.
    ///
    /// @return A constant reference to the local rotation.
    const glm::vec3& GetRotation() const;

    /// @brief Gets the Node's local scale.
    ///
    /// @return A constant reference to the local scale.
    const glm::vec3& GetScale() const;

    /// @brief Gets the local transformation matrix.
    ///
    /// @return A constant reference to the local transformation matrix.
    const glm::mat4& GetLocalTransform() const;

    /// @brief Gets the world transformation matrix.
    ///
    /// @return A constant reference to the world transformation matrix.
    const glm::mat4& GetWorldTransform() const;

private:
    std::string m_name;

    std::weak_ptr<Node> m_parent;
    std::vector<std::shared_ptr<Node>> m_children;

    // Scene

    friend class Scene;

    /// @brief Sets the Scene that this Node belongs to.
    ///
    /// The Scene reference is also propagated to all child Nodes.
    ///
    /// @param p_scene Scene that owns this Node.
    void SetScene(std::shared_ptr<Scene> p_scene);

    /// @brief Weak reference to the Scene containing this Node.
    ///
    /// The Scene owns the Node hierarchy, so the Node stores only a
    /// weak reference to avoid creating an ownership cycle.
    std::weak_ptr<Scene> m_scene;

    // Components

    std::vector<std::shared_ptr<Component>> m_components;

    // Transforms

    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 m_localTransform{ 1.0f };
    glm::mat4 m_worldTransform{ 1.0f };

    bool m_active = true;

    /// @brief Indicates whether the Node has been started.
    bool m_started = false;
};