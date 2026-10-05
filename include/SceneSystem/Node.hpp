#pragma once

#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <Transform.hpp>
#include "BoundingBox.hpp"

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
        /// Constructs the Component using the provided arguments, assigns this Node
        /// as its owner, and stores it.
        ///
        /// If the Component provides a bounds override, it is registered as the
        /// Node's bounds provider. A Node can only have one Component providing
        /// a bounds override.
        ///
        /// If the Node has already been started, the Component is started immediately.
        ///
        /// Multiple Components of the same type can be attached to a Node.
        ///
        /// @tparam T Type of Component to create.
        /// @tparam Args Types of arguments used to construct the Component.
        ///
        /// @param p_args Arguments forwarded to the Component constructor.
        ///
        /// @return A shared pointer to the newly created Component.
        ///
        /// @throws std::runtime_error if the Component provides a bounds override
        /// and another bounds override is already registered with this Node.
        template<typename T, typename... Args>
        std::shared_ptr<T> AddComponent(Args&&... p_args)
        {
            // Ensure that T is derived from Component
            static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component.");

            auto component = std::make_shared<T>(std::forward<Args>(p_args)...);

            component->SetOwner(shared_from_this());

            m_components.push_back(component);

            if (component->HasBoundsOverride())
            {
                if (m_boundsOverride)
                {
                    throw std::runtime_error("Node already has a bounds override.");
                }
                m_boundsOverride = component;
            }

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

        // --------------------------------------------------
        // Bounds
        // --------------------------------------------------

        /// @brief Gets the Node's effective local-space bounding box.
        ///
        /// Returns the bounds provided by a Component if a bounds override is
        /// registered. Otherwise, returns the Node's default bounding box.
        ///
        /// @return A constant reference to the effective local-space bounding box.
        [[nodiscard]] const BoundingBox &GetBounds() const;

        /// @brief Sets the Node's default local-space bounding box.
        ///
        /// The specified bounds are used when no Component provides a bounds
        /// override.
        ///
        /// @param p_bounds Local-space bounding box to use as the Node's default.
        void SetBounds(const BoundingBox &p_bounds);

        /// @brief Gets the Node's effective world-space bounding box.
        ///
        /// The Node's effective local-space bounds are transformed using the Node's
        /// world transform.
        ///
        /// @return The effective bounding box in world space.
        [[nodiscard]] BoundingBox GetWorldBounds() const;

        /// @brief Checks whether bounds checks are enabled for the Node.
        ///
        /// @return true if the Node should participate in bounds checks, otherwise false.
        [[nodiscard]] bool IsBoundsEnabled() const;

        /// @brief Enables or disables bounds checks for the Node.
        ///
        /// When disabled, the Node is ignored by systems that use Node bounds for
        /// collision or raycast checks.
        ///
        /// @param p_enabled true to enable bounds checks, false to disable them.
        void SetBoundsEnabled(bool p_enabled);

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

        // Bounds

        /// @brief Default local-space bounding box for this Node.
        ///
        /// Used when no Component provides a bounds override.
        BoundingBox m_bounds{};

        /// @brief Indicates whether this Node participates in bounds checks.
        bool m_boundsEnabled = true;

        /// @brief Component currently providing the Node's bounds override.
        ///
        /// Only one Component can provide a bounds override at a time.
        std::shared_ptr<Component> m_boundsOverride{};
    };
}