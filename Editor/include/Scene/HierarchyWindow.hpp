#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"
#include <Engine/Engine.hpp>
#include <memory>
#include <utility>
#include <vector>

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

            void RefreshLoadedScenes(std::vector<std::string>&p_sceneFiles );

            void RefreshLoadedNodes(std::vector<std::string> &p_nodeFiles);

            void DrawSceneWindow();

            /// @brief Draws a scene node and its children.
            ///
            /// Recursively traverses the node hierarchy and creates an ImGui tree
            /// entry for each node. Handles node selection, drag-and-drop
            /// reparenting, and the node context menu.
            ///
            /// @param p_node The node to draw.
            void DrawNode(const std::shared_ptr<Droplet::Scene::Node> &p_node); // TODO: Make weak pointer

            /// @brief Handles rectangle (marquee) selection that is started in the empty area of the hierarchy.
            /// @param p_emptyAreaMin Top-left of the node area, in screen space.
            /// @param p_emptyAreaMax Bottom-right of the node area, in screen space.
            void HandleRectSelection(const ImVec2 &p_emptyAreaMin, const ImVec2 &p_emptyAreaMax);

            /// @brief Checks whether a screen-space position is over a node row drawn this frame.
            [[nodiscard]] bool IsOverNodeRow(const ImVec2 &p_pos) const;

            /// @brief Nodes drawn this frame in display order, used for shift-click range selection.
            std::vector<std::shared_ptr<Droplet::Scene::Node>> m_visibleNodes; // TODO: Make weak pointer

            /// @brief Screen-space row rectangles (min, max) matching m_visibleNodes, used for rectangle selection.
            std::vector<std::pair<ImVec2, ImVec2>> m_visibleNodeRects;

            /// @brief Whether a rectangle selection drag is in progress in the empty area of the hierarchy.
            bool m_marqueeActive = false;

            /// @brief Start of the rectangle selection in scroll-independent window content coordinates.
            ImVec2 m_marqueeStart{};

            /// @brief The node that was last clicked without shift, used as the anchor for range selection.
            std::weak_ptr<Droplet::Scene::Node> m_selectionAnchor;

            /// @brief Node that was shift-clicked this frame, resolved after all nodes are drawn.
            std::shared_ptr<Droplet::Scene::Node> m_rangeSelectTarget; // TODO: Make weak pointer

            /// @brief Already-selected node that was pressed without modifiers. The selection collapses to it on release, unless dragged.
            std::shared_ptr<Droplet::Scene::Node> m_pendingCollapseNode; // TODO: Make weak pointer

            /// @brief Whether the pending range selection should be added to the existing selection (ctrl+shift).
            bool m_rangeSelectAdditive = false;

            /// @brief Engine instance used to access engine systems.
            std::shared_ptr<Engine> m_instance; // TODO: Make weak pointer
            
            /// @brief EditorContext that is used to access information from a scene between
            /// different views
            EditorContext *m_editorContext;
            
			/// @brief The currently selected scene in the hierarchy window.
			std::shared_ptr<Droplet::Scene::Scene> m_selectedScene; // TODO: Make weak pointer

            /// @brief Nodes that should be reparented after hierarchy traversal.
            ///
            /// Hierarchy modifications are deferred until the hierarchy has been
            /// fully rendered to avoid modifying node containers while they are
            /// being traversed.
            std::vector<std::shared_ptr<Droplet::Scene::Node>> m_nodesToReparent; // TODO: Make weak pointer

            /// @brief New parent for the node being reparented.
            ///
            /// A null value indicates that the node should become a root node.
            std::shared_ptr<Droplet::Scene::Node> m_newParent; // TODO: Make weak pointer

            /// @brief Node that should receive a newly created child.
            ///
            /// The child is created and added after the hierarchy has been fully
            /// rendered to avoid modifying the hierarchy while it is being traversed.
            std::shared_ptr<Droplet::Scene::Node> m_nodeToAddChildTo; // TODO: Make weak pointer

            std::vector<std::shared_ptr<Droplet::Scene::Node>> m_nodesToRemove; // TODO: Make weak pointer
        };
    }
}
