#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"

#include <memory>

namespace Droplet
{
    class DropletInstance;

    namespace Scene
    {
        class Node;
		class SceneManager;
    }
    namespace Editor
    {
        class HierarchyWindow : public EditorWindow
        {
        public:
            explicit HierarchyWindow(std::shared_ptr<DropletInstance> p_instance, std::shared_ptr<InteractionState> p_interactionState);
              
        protected:
            void InitImpl() override;
            void RenderImpl() override;

        private:
            void DrawNode(const std::shared_ptr<Scene::Node> &p_node);
            std::shared_ptr<DropletInstance> m_instance;
            std::shared_ptr<InteractionState> m_interactionState;

            std::shared_ptr<Scene::SceneManager> m_testSceneManager;
        };
    }
}