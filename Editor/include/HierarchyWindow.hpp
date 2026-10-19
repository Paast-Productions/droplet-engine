#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"
#include <Engine/Engine.hpp>
#include <memory>

#include "EditorContext.hpp"

namespace Droplet
{
    namespace Scene
    {
		// Forward declarations
        class Node;
        class Scene;
        class SceneManager;
    }

    namespace Editor
    {
        /// @brief Editor window used to display the scene node hierarchy.
        /// 
        /// Displays the nodes belonging to the currently viewed scene and
        /// allows nodes to be selected through the hierarchy.
        class HierarchyWindow : public EditorWindow   
        {
        public:
            /// @brief Constructs a hierarchy window.
            /// @param p_instance The instance of the engine used to access scene data.
            /// @param p_interactionState The shared interaction state used to track selections.
            explicit HierarchyWindow(EditorContext *p_editorContext);

        protected:
            /// @brief Initializes the hierarchy window.
            /// 
            /// Creates temporary scene data used to test the hierarchy window.
            void InitImpl() override;

            /// @brief Renders the hierarchy window.
            /// 
            /// Displays the node hierarchy of the currently viewed scene.
            void RenderImpl() override;

        private:
            /// @brief Draws a scene node and its children.
            ///
            /// Recursively traverses the node hierarchy and creates an ImGui tree
            /// entry for each node. Handles node selection, drag-and-drop
            /// reparenting, and the node context menu.
            ///
            /// @param p_node The node to draw.
            void DrawNode(const std::shared_ptr<Droplet::Scene::Node> &p_node); // TODO: Make weak pointer

            /// @brief Engine instance used to access engine systems.
            std::shared_ptr<Engine> m_instance; // TODO: Make weak pointer
            
            /// @brief EditorContext that is used to access information from a scene between
            /// different views
            EditorContext *m_editorContext;
            
			/// @brief The currently selected scene in the hierarchy window.
			std::shared_ptr<Droplet::Scene::Scene> m_selectedScene; // TODO: Make weak pointer

            /// @brief Node that should be reparented after hierarchy traversal.
            ///
            /// Hierarchy modifications are deferred until the hierarchy has been
            /// fully rendered to avoid modifying node containers while they are
            /// being traversed.
            std::shared_ptr<Droplet::Scene::Node> m_nodeToReparent; // TODO: Make weak pointer

            /// @brief New parent for the node being reparented.
            ///
            /// A null value indicates that the node should become a root node.
            std::shared_ptr<Droplet::Scene::Node> m_newParent; // TODO: Make weak pointer

            /// @brief Node that should receive a newly created child.
            ///
            /// The child is created and added after the hierarchy has been fully
            /// rendered to avoid modifying the hierarchy while it is being traversed.
            std::shared_ptr<Droplet::Scene::Node> m_nodeToAddChildTo; // TODO: Make weak pointer

            std::shared_ptr<Droplet::Scene::Node> m_nodeToRemove; // TODO: Make weak pointer
        };
    }
}
