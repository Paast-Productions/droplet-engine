#pragma once

#include <json/json.hpp>
#include <memory>
#include <string_view>
#include <SceneSystem/bounds/NodeBounds.hpp>
#include <Graphics/RenderData.hpp>

namespace Droplet::Scene
{
    // Forward declaration
    class Node;
    class SerializationContext;

    /// @brief Base class for components that can be attached to a Node.
    ///
    /// Components provide reusable functionality that can be added to Nodes
    /// at runtime. Derived components can override Initialize(), OnStart(),
    /// and Update() to implement their own behavior.
    ///
    /// Components are owned by the Node they are attached to. A Component
    /// maintains a weak reference to its owning Node to avoid an ownership cycle.
    class Component : public std::enable_shared_from_this<Component>
    {
    public:
		Component() = default;
		virtual ~Component() = default;

        Component(const Component &) = delete;			    // Copy constructor
        Component(Component &&) = delete;					// Move constructor
        Component &operator=(const Component &) = delete;	// Copy assignment operator
        Component &operator=(Component &&) = delete;		// Move assignment operator
        // Copy & Move is disabled to ensure pointers always remain valid
        // Copying/Moving a Component should be explicit and manual

		/// @brief Gets the type name of the component. 
        /// 
        /// This function must be overridden by derived classes to return the correct type name, 
        /// as it is used for serialization and identification of component types.
        /// @return The type name of the component.
        virtual std::string_view GetTypeName() const = 0;

        /// @brief Called when the owning Node starts.
        ///
        /// Called when the owning Node starts participating in an active Scene.
        /// This function is called at most once for each Component.
        /// Derived classes can override this function to perform startup logic.
        void Start();

        /// @brief Updates the component.
        ///
        /// Called once per update for components attached to active Nodes.
    	void Update();

    	/// @brief Late updates the component.
    	///
    	/// Called once per update for components attached to active Nodes.
    	void LateUpdate();

    	/// @brief Updates the component in fixed time.
    	void FixedUpdate();

		/// @brief Updates the component in editor mode, regardless of wether in play mode or not.
        ///
        /// Called once per update for components attached to active Nodes.
    	void EditorUpdate();

		/// @brief Notifies the scene that the component wants to be rendered this frame.
		/// 
        /// Cameras use this to request a render pass for their view. 
		/// Renderable components use this to notify cameras that they should be rendered.
        void QueueRender();

		/// @brief Retrieves any resources needed for rendering the component.
		/// @return A RenderData object containing the component's rendering data.
        [[nodiscard]] Droplet::Graphics::RenderData GetRenderData();

		/// @brief Called to render the default component UI wrapper as well as the overloaded RenderUIImpl() function.
        void RenderUI();

        /// @brief Gets the Node that owns this component.
        /// @return A shared pointer to the owning Node, or nullptr if the
        /// owner no longer exists.
        [[nodiscard]] std::shared_ptr<Node> GetOwner() const; // TODO: Make weak pointer
    
		/// @brief Serializes the component to a JSON object.
		/// @return A JSON object representing the component's state.
		[[nodiscard]] nlohmann::json Serialize();

		/// @brief Deserializes the component from a JSON object.
		/// @param p_compJson A JSON object containing the component's state.
		void Deserialize(const nlohmann::json p_compJson);

        /// @brief Determines whether the component provides custom bounds.
        ///
        /// Components normally use the bounding box provided by their Node.
        /// Components that provide their own bounds, such as a MeshComponent,
        /// should override this function and return true.
        ///
        /// @return True if the component provides custom bounds, otherwise false.
        virtual bool HasBoundsOverride() const
        {
            return false;
        }

        /// @brief Gets the custom bounds provided by the component.
        ///
        /// This function is used by components that provide bounds that are
        /// more specific than the Node's default bounding box. For example,
        /// a MeshComponent can provide bounds calculated from its mesh.
        ///
        /// The default implementation returns a default BoundingBox and should
        /// only be used when HasBoundsOverride() returns false.
        ///
        /// @return The bounding box provided by the component.
        [[nodiscard]] virtual const NodeBounds *GetBounds() const { return nullptr; }

		/// @brief Checks if the component and all of its ancestors are active.
		/// @return true if the component and all of its ancestors are active, otherwise false.
		/// @throws std::runtime_error if the component has no owner.
        [[nodiscard]] bool IsActive() const;

		/// @brief Check if the component itself is active.
		/// @return A shared pointer to the owning Node, or nullptr if the
        [[nodiscard]] bool IsActiveSelf() const { return m_active; }

		/// @brief Sets whether the component itself is active.
		/// @param p_active true to activate the component, false to deactivate it.
        void SetActiveSelf(bool p_active);

		/// @brief Called when the component becomes active or inactive. 
        /// 
		/// Note: This is called the active stack changes, not just it's internal active state. 
		/// For example, if the owning node is already inactive, setting this component to active or inactive 
		/// will not trigger this function, as the effective active state of the component has not changed.
		/// @param p_active true if the component is now active, false if it is now inactive.
		void ActiveChanged(const bool p_active);
        
        /// @brief Retrieves the components ID
        /// @return std::uint64_t m_id
        std::uint32_t GetID() const { return m_id; }


        /// @brief Finds the correct node for the component with it's serializedID
        /// 
        /// This functions is used by components when they are deserialized,
        /// after all nodes and components are created we lookup what id they had so the newly 
        /// created node/component can be set as a target and more easily referenced
        /// 
        /// @param p_context The context that holds the mapping of newly deserialized nodes and components
        virtual void ResolveReferences([[maybe_unused]]SerializationContext &p_context);

    protected:

        virtual void StartImpl() {}

        virtual void UpdateImpl() {}
    	
    	virtual void LateUpdateImpl() {}
    	
    	virtual void FixedUpdateImpl() {}

        virtual void EditorUpdateImpl() {}
    	
    	virtual void ActiveChangedImpl(const bool p_active) {}

        virtual void QueueRenderImpl() {}

        [[nodiscard]] virtual Droplet::Graphics::RenderData GetRenderDataImpl() { return {}; }

		/// @brief Internal rendering function for the component's UI.
        /// 
		/// Overloaded by derived components to implement their own UI rendering logic.
        virtual void RenderUIImpl() {}

		/// @brief Internal serialization function for the component's state.
        /// 
		/// Overloaded by derived components to implement their own serialization logic.
		/// @return A JSON object representing the component's state.
        [[nodiscard]] virtual nlohmann::json SerializeImpl() { return {}; }

		/// @brief Internal deserialization function for the component's state.
        /// 
		/// Overloaded by derived components to implement their own deserialization logic.
		/// @param p_compJson A JSON object containing the component's state.
        virtual void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) { }

        /// @brief Sets the component to be renderable
        [[nodiscard]] void SetRenderable() { m_renderable = true; }

    private:
        friend class Node;

		bool m_started = false;
		bool m_active = true;
        bool m_renderable = false;

        std::weak_ptr<Node> m_owner{};
        std::uint64_t m_id = 0;
        /// @brief Sets the Node that owns this component.
        ///     
        /// Only Node can set the owner of a Component. This is used when
        /// the component is added to a Node.
        ///
        /// @param p_owner Node that will own the component.
        void SetOwner(std::shared_ptr<Node> p_owner); // TODO: Make weak pointer
    };
}
