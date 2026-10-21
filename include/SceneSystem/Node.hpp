#pragma once

#include <stdexcept>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <atomic>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Transform.hpp>
#include <SceneSystem/bounds/NodeBounds.hpp>
#include <Graphics/RenderData.hpp>

namespace Droplet::Scene
{

    // TODO: Refactor after Material implementation
    struct NodeData
    {
        std::uint32_t meshID;
        std::uint32_t textureID;

        glm::mat4 world;
    };

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
        explicit Node(const std::shared_ptr<Scene> &p_scene, const std::string &p_name); // TODO: Make weak pointer

        ~Node() = default;

		Node() = delete;                        // Default constructor
        Node(const Node &) = delete;			// Copy constructor
        Node(Node &&) = delete;					// Move constructor
        Node &operator=(const Node &) = delete;	// Copy assignment operator
        Node &operator=(Node &&) = delete;		// Move assignment operator
        // Copy & Move is disabled to ensure pointers always remain valid
        // Copying/Moving a Node should be explicit and manual

        /// @brief Starts the Node, its Components, and its children.
        ///
        /// Calls OnStart() on all Components attached to this Node and then
        /// starts all child Nodes.
        ///
        /// OnStart() is called at most once for each Node. Nodes that are
        /// added to an already started Node are started immediately.
        ///
        /// @note The Node's Start() function can be overridden by derived Nodes.
        void Start();

        /// @brief Updates the Node, its Components, and its children.
        ///
        /// Updates the Node's transforms, followed by its Components
        /// and child Nodes.
        void Update();

        /// @brief Updates the Node, its Components, and its children late.
        void LateUpdate();

        /// @brief Updates the Node, its Components, and its children in fixed time.
        void FixedUpdate();

        /// @brief Renders the Node.
        ///
        /// @note Currently unused and does not perform any rendering.
        /// Rendering functionality may be moved.
        void GetRenderData(std::vector<Graphics::RenderData> &p_renderData);

		/// @brief Renders the Node's UI as well as all attached Components.
        void RenderUI();

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
        [[nodiscard]] Transform &GetTransform() { return m_transform; } // TODO: Make weak pointer
		[[nodiscard]] const Transform &GetTransform() const { return m_transform; } // TODO: Make weak pointer

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
        std::shared_ptr<Node> AddChild(std::shared_ptr<Node> p_child); // TODO: Make weak pointer

        /// @brief Removes a child Node.
        ///
        /// Removes the specified Node from the hierarchy and clears its
        /// parent reference.
        ///
        /// @param p_child Node to remove.
        ///
        /// @throws std::invalid_argument if p_child is nullptr.
        /// @throws std::runtime_error if the Node is not a child of this Node.
        void RemoveChild(const std::shared_ptr<Node> &p_child); // TODO: Make weak pointer

        /// @brief Gets the parent Node.
        ///
        /// @return A shared pointer to the parent Node, or nullptr if this
        /// Node has no parent.
        [[nodiscard]] std::shared_ptr<Node> GetParent() const; // TODO: Make weak pointer

        /// @brief Gets the child Nodes.
        ///
        /// @return A constant reference to the vector containing this Node's children.
        [[nodiscard]] const std::vector<std::shared_ptr<Node>> &GetChildren() const; // TODO: Make weak pointer

        /// @brief Gets the Scene that owns this Node.
        ///
        /// @return A shared pointer to the Scene, or nullptr if the Node
        /// does not currently belong to a Scene.
        [[nodiscard]] std::shared_ptr<Scene> GetScene() const; // TODO: Make weak pointer

        // --------------------------------------------------
        // Name
        // --------------------------------------------------

        /// @brief Gets the Node's name.
        ///
        /// @return A constant reference to the Node's name.
        [[nodiscard]] const std::string &GetName() const;

        /// @brief Sets the Node's name.
        void SetName(const std::string &p_name);

        // --------------------------------------------------
        // ID
        // --------------------------------------------------

        [[nodiscard]] std::uint64_t GetID() const noexcept
        {
            return m_id;
        }

        // --------------------------------------------------
        // Components
        // --------------------------------------------------

		/// @brief Adds a Component to the Node by taking ownership of a unique pointer.
		/// @param p_component Unique pointer to the Component to add.
        void AddComponent(std::unique_ptr<Component> &p_component);

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

        /// @brief Adds an exisiting component to the node
        /// 
        /// Attached an already created comoponent of any type
        /// adds it to the member vector of component the node owns.
        /// It assigns a freshly new component id
        /// The overloaded function uses id from serialization
        ///       
        /// @param p_component The comoponent to be attached
        void AttachComponent(std::unique_ptr<Component> &p_component); 
        void AttachComponent(std::shared_ptr<Component> &p_component, std::uint64_t p_componentID);

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
        [[nodiscard]] std::vector<std::shared_ptr<T>> GetComponents() const // TODO: Make weak pointer
        {
            // Ensure that T is derived from Component
            static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component.");

            std::vector<std::shared_ptr<T>> components; // TODO: Make weak pointer

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


        
        /// @brief Gets all components 
        /// 
        /// @return A vector of shared pointers to the components, the member variable of this class
        [[nodiscard]] std::vector<std::shared_ptr<Component>> GetAllComponents() const; // TODO: Make weak pointer

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
        void RemoveComponent(const Component *p_component);

        // --------------------------------------------------
        // Bounds
        // --------------------------------------------------

        /// @brief Gets the Node's effective local-space bounding box.
        ///
        /// Returns the bounds provided by a Component if a bounds override is
        /// registered. Otherwise, returns the Node's default bounding box.
        ///
        /// @return A constant reference to the effective local-space bounding box.
        [[nodiscard]] const NodeBounds *GetBounds() const;

        /// @brief Sets the Node's default local-space bounding box.
        ///
        /// The specified bounds are used when no Component provides a bounds
        /// override.
        ///
        /// @param p_bounds Local-space bounding box to use as the Node's default.
        void SetBounds(std::shared_ptr<NodeBounds> p_bounds); // TODO: Make weak pointer

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

		/// @brief Checks if this Node is a descendant of the specified Node.
        ///
		/// @param p_node Node to check against.
        /// 
		/// @return true if this Node is a descendant of p_node, otherwise false.
        [[nodiscard]] bool IsDescendantOf(const std::shared_ptr<Node> &p_node) const; // TODO: Make weak pointer

		/// @brief Checks through all components to see if the Node is renderable.
        /// 
        /// @return true if the Node is renderable, otherwise false.
        [[nodiscard]] bool IsRenderable() const;

    private:
        std::string m_name;
        
        inline static std::atomic<std::uint64_t> s_nextID{1};
        const std::uint64_t m_id;

        std::uint64_t m_nextComponentID = 1;
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
        void SetScene(std::shared_ptr<Scene> p_scene); // TODO: Make weak pointer

        // Components

        std::vector<std::shared_ptr<Component>> m_components{};

        // Bounds

        /// @brief Default local-space bounding box for this Node.
        ///
        /// Used when no Component provides a bounds override.
        std::shared_ptr<NodeBounds> m_bounds{};

        /// @brief Indicates whether this Node participates in bounds checks.
        bool m_boundsEnabled = true;

        /// @brief Component currently providing the Node's bounds override.
        ///
        /// Only one Component can provide a bounds override at a time.
        std::weak_ptr<Component> m_boundsOverride{};
    };
}