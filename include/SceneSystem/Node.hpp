#pragma once

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Transform.hpp>

namespace Droplet::Scene
{
    /// Forward declarations to avoid circular dependencies
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
        /// @brief Constructs a Node with the specified name and Scene.
		/// @param p_scene Shared pointer to the Scene that owns this Node.
        /// @param p_name Name of the Node.
        explicit Node(std::shared_ptr<Scene> p_scene, const std::string &p_name);

        /// @brief Virtual destructor.
        virtual ~Node() = default;

        /// @brief Starts the Node, its Components, and its children.
        ///
        /// Calls OnStart() on all Components attached to this Node and then
        /// starts all child Nodes.
        ///
        /// OnStart() is called at most once for each Node. Nodes that are
        /// added to an already started Node are started immediately.
        ///
        /// @note The Node's Start() function can be overridden by derived Nodes.
        virtual void Start();

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

		/// @brief Renders the Node's UI as well as all attached Components.
        virtual void RenderUI();

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
        [[nodiscard]] bool IsActive() const;

		/// @brief Checks whether the Node is active.
        ///
		/// @return true if the Node is active, otherwise false.
        [[nodiscard]] bool IsActiveSelf() const;

        // --------------------------------------------------
        // Transform
        // --------------------------------------------------

		/// @brief Gets the Node's Transform.
		/// @return A reference to the Node's Transform.
        [[nodiscard]] Transform &GetTransform() { return m_transform; }
		[[nodiscard]] const Transform &GetTransform() const { return m_transform; }

        // --------------------------------------------------
        // Children
        // --------------------------------------------------

        /// @brief Adds a child Node.
        ///
        /// Sets this Node as the parent of the specified child and adds
        /// the child to the Node hierarchy.
        ///
        /// @param p_child Node to add as a child.
        /// @return A shared pointer to the added child Node.
        ///
        /// @throws std::invalid_argument if p_child is nullptr.
        /// @throws std::runtime_error if the Node already has a parent
        /// or belongs to a Scene.
        std::shared_ptr<Node> AddChild(std::shared_ptr<Node> p_child);

        /// @brief Removes a child Node.
        ///
        /// Removes the specified Node from the hierarchy and clears its
        /// parent reference.
        ///
        /// @param p_child Node to remove.
        ///
        /// @throws std::invalid_argument if p_child is nullptr.
        /// @throws std::runtime_error if the Node is not a child of this Node.
        void RemoveChild(const std::shared_ptr<Node> &p_child);

        /// @brief Gets the parent Node.
        ///
        /// @return A shared pointer to the parent Node, or nullptr if this
        /// Node has no parent.
        [[nodiscard]] std::shared_ptr<Node> GetParent() const;

        /// @brief Gets the child Nodes.
        ///
        /// @return A constant reference to the vector containing this Node's children.
        [[nodiscard]] const std::vector<std::shared_ptr<Node>> &GetChildren() const;

        /// @brief Gets the Scene that owns this Node.
        ///
        /// @return A shared pointer to the Scene, or nullptr if the Node
        /// does not currently belong to a Scene.
        [[nodiscard]] std::shared_ptr<Scene> GetScene() const;

        // --------------------------------------------------
        // Name
        // --------------------------------------------------

        /// @brief Gets the Node's name.
        ///
        /// @return A constant reference to the Node's name.
        [[nodiscard]] const std::string &GetName() const;



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
            // Ensure that T is derived from Component
            static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component.");

            auto component = std::make_shared<T>(std::forward<Args>(p_args)...);

            component->SetOwner(shared_from_this());

            m_components.push_back(component);

            if (m_started)
            {
                component->Start();
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
        [[nodiscard]] std::vector<std::shared_ptr<T>> GetComponents() const
        {
            // Ensure that T is derived from Component
            static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component.");

            std::vector<std::shared_ptr<T>> components;

            for (const auto &component : m_components)
            {
                auto result = std::dynamic_pointer_cast<T>(component);

                if (result)
                {
                    components.push_back(result);
                }
            }

            return components;
        }

        /// @brief Removes a specific Component from the Node.
        ///
        /// Removes the specified Component from the Node's list of Components.
        /// Components are removed by instance, allowing multiple Components
        /// of the same type to exist on the same Node.
        ///
        /// @param p_component Component to remove.
        ///
        /// @throws std::invalid_argument if p_component is nullptr.
        /// @throws std::runtime_error if the Component is not attached to this Node.
        void RemoveComponent(const std::shared_ptr<Component> &p_component);


    private:
        std::string m_name;

		/// @brief Indicates whether the Node is active.
        bool m_active = true;

        /// @brief Indicates whether the Node has been started.
        bool m_started = false;

		// Transform

        Transform m_transform;

		// Hierarchy

        std::weak_ptr<Node> m_parent;
        std::vector<std::shared_ptr<Node>> m_children;

        // Scene

        friend class Scene;

        /// @brief Weak reference to the Scene containing this Node.
        ///
        /// The Scene owns the Node hierarchy, so the Node stores only a
        /// weak reference to avoid creating an ownership cycle.
        std::weak_ptr<Scene> m_scene{};

        /// @brief Sets the Scene that this Node belongs to.
        ///
        /// The Scene reference is also propagated to all child Nodes.
        ///
        /// @param p_scene Scene that owns this Node.
        void SetScene(std::shared_ptr<Scene> p_scene);

        // Components

        std::vector<std::shared_ptr<Component>> m_components{};
    };
}