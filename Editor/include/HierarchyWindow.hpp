#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"
#include <Engine/Engine.hpp>
#include <memory>

namespace Droplet
{
    namespace Scene
    {
        class Node;
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
            explicit HierarchyWindow(std::shared_ptr<Droplet::Engine> p_instance, std::shared_ptr<InteractionState> p_interactionState);

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
            /// Recursively traverses the node hierarchy and creates
            /// an ImGui tree entry for each node.
            /// 
            /// @param p_node The node to draw.
            void DrawNode(const std::shared_ptr<Droplet::Scene::Node> &p_node);

            /// @brief Engine instance used to access engine systems.
            std::shared_ptr<Engine> m_instance;

            /// @brief Shared editor interaction state used to track node selection.
            std::shared_ptr<InteractionState> m_interactionState;

            /// @brief Temporary scene manager used for testing the hierarchy window.
            std::shared_ptr<Droplet::Scene::SceneManager> m_testSceneManager;
        };
    }
}