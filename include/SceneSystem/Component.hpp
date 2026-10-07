#pragma once

#include <memory>

namespace Droplet::Scene
{
    class Node; // Forward declaration

    /// @brief Base class for components that can be attached to a Node.
    ///
    /// Components provide reusable functionality that can be added to Nodes
    /// at runtime. Derived components can override Initialize(), OnStart(),
    /// and Update() to implement their own behavior.
    ///
    /// Components are owned by the Node they are attached to. A Component
    /// maintains a weak reference to its owning Node to avoid an ownership cycle.
    class Component
    {
    public:
        /// @brief Constructs an empty Component.
        Component();

        /// @brief Virtual destructor.
        virtual ~Component() = default;

        /// @brief Called when the owning Node starts.
        ///
        /// Called when the owning Node starts participating in an active Scene.
        /// This function is called at most once for each Component.
        /// Derived classes can override this function to perform startup logic.
        virtual void Start() {}

        /// @brief Updates the component.
        ///
        /// Called once per update for components attached to active Nodes.
        ///
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
        virtual void Update([[maybe_unused]] float p_deltaTime) {}

		/// @brief Called to render the default component UI wrapper as well as the overloaded RenderUI() function.
        void RenderUI();

        /// @brief Gets the Node that owns this component.
        ///
        /// @return A shared pointer to the owning Node, or nullptr if the
        /// owner no longer exists.
        std::shared_ptr<Node> GetOwner() const;

		/// @brief Checks if the component and all of its ancestors are active.
		/// @return true if the component and all of its ancestors are active, otherwise false.
		/// @throws std::runtime_error if the component has no owner.
        [[nodiscard]] bool IsActive() const;

		/// @brief Check if the component itself is avtive.
		/// @return A shared pointer to the owning Node, or nullptr if the
        [[nodiscard]] bool IsActiveSelf() const { return m_active; }

		/// @brief Sets whether the component itself is active.
		/// @param p_active true to activate the component, false to deactivate it.
        void SetActiveSelf(bool p_active);

    protected:

		/// @brief Internal rendering function for the component's UI.
		/// Overloaded by derived components to implement their own UI rendering logic.
        virtual void RenderInternalUI() {}

    private:
        friend class Node;

		bool m_active = true;

        std::weak_ptr<Node> m_owner{};

        /// @brief Sets the Node that owns this component.
        ///     
        /// Only Node can set the owner of a Component. This is used when
        /// the component is added to a Node.
        ///
        /// @param p_owner Node that will own the component.
        void SetOwner(std::shared_ptr<Node> p_owner);
    };
}