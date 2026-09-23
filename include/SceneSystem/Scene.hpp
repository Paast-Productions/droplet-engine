#pragma once

#include <memory>
#include <string>
#include <vector>


namespace Droplet::Scene
{
    // forward declaration of Node to avoid circular dependency
    class Node;

    /// @brief Represents a Scene containing one or more Node hierarchies.
    ///
    /// A Scene owns any number of root Nodes that serve as the starting
    /// points of the Scene's Node hierarchies. Scenes can be loaded,
    /// unloaded, activated, and updated independently.
    ///
    /// Nodes maintain a weak reference to their containing Scene so
    /// that they can access Scene-level functionality without creating
    /// an ownership cycle.
    class Scene : public std::enable_shared_from_this<Scene>
    {
    public:

        /// @brief Constructs a Scene with the specified name.
        /// @param p_name Name of the Scene.
        explicit Scene(const std::string &p_name);

        /// @brief Virtual destructor.
        virtual ~Scene() = default;      

        /// @brief Loads the Scene.
        ///
        /// Marks the Scene as loaded and assigns the Scene reference to
        /// all root Nodes. The Scene reference is propagated through
        /// each Node hierarchy.
        ///
        /// Loading a Scene does not activate it.
        ///
        /// @throws std::runtime_error if the Scene is already loaded.
        ///
        /// Derived Scenes can override this function to perform additional
        /// loading or initialization.
        virtual void Load();

        /// @brief Unloads the Scene.
        ///
        /// Marks the Scene as unloaded and deactivates it.
        ///
        /// The root Nodes and their hierarchies remain owned by the Scene
        /// and can be loaded again later.
        ///
        /// @throws std::runtime_error if the Scene is not loaded.
        ///
        /// Derived Scenes can override this function to perform additional
        /// cleanup.
        virtual void Unload();

        /// @brief Updates the Scene.
        ///
        /// Updates all root Node hierarchies when the Scene is both
        /// loaded and active.
        ///
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
        virtual void Update(float p_deltaTime);

        /// @brief Renders the Scene.
        ///
        /// Renders all root Node hierarchies when the Scene is both
        /// loaded and active.
        /// 
        /// @note Currently unused and does not perform any rendering.
        /// Rendering functionality may be moved to a separate rendering
        /// system in the future.
        virtual void Render();

        /// @brief Creates a Node as a root Node of the Scene.
        ///
        /// If the Scene is already active, the Node is started immediately.
        ///
        /// @param p_name Name of the Node to create.
        ///
        /// @return A shared pointer to the created Node.
        std::shared_ptr<Node> AddNode(const std::string &p_name);

		/// @brief Sets whether a Node is a root Node of the Scene.
		/// @param p_node Node to set as a root Node.
		/// @param p_makeRoot true to set the Node as a root Node, false to remove it from the root Nodes.
		void SetRoot(const std::shared_ptr<Node> &p_node, bool p_makeRoot);

        /// @brief Removes a root Node from the Scene.
        ///
        /// The Node is removed from the Scene's root Node collection.
        ///
        /// Removing a root Node does not destroy it if other shared
        /// pointers to the Node exist.
        ///
        /// @param p_node Root Node to remove.
        ///
        /// @throws std::invalid_argument if p_node is nullptr.
        /// @throws std::runtime_error if the Node is not a root of this Scene.
        void RemoveRoot(const std::shared_ptr<Node> &p_node);

        /// @brief Gets the root Nodes of the Scene.
        ///
        /// Root Nodes are Nodes that do not have a parent Node and serve as
        /// the starting points of the Scene's Node hierarchies. A Scene can
        /// contain any number of root Nodes.
        ///
        /// @return A constant reference to the Scene's root Nodes.
        const std::vector<std::shared_ptr<Node>> &GetRoots() const;

        /// @brief Gets the Scene's name.
        ///
        /// @return A constant reference to the Scene's name.
        const std::string &GetName() const;

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
        /// Activating a loaded Scene starts all root Nodes and their
        /// hierarchies. An active Scene is updated and rendered by the
        /// SceneManager.
        ///
        /// Deactivating a Scene prevents its Node hierarchies from being
        /// updated or rendered.
        ///
        /// @param p_active true to activate the Scene, false to deactivate it.
        ///
        /// @throws std::runtime_error if attempting to activate a Scene
        /// that has not been loaded.
        void SetActive(bool p_active);

    private:
        std::string m_name;
        std::vector<std::shared_ptr<Node>> m_roots;

        bool m_loaded = false;
        bool m_active = false;
    };     
}