
#pragma once

#include <SceneSystem/Behaviour.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Droplet::Scene
{
    class Scene;
    class TransitionComponent;

    class TransitionBehaviour : public Behaviour
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() override
        {
            return "TransitionBehaviour";
        }

        bool IsSingleton() override
        {
            return false;
        }

        /// Registers a transition component.
        void AddTransitionComponent(
            std::weak_ptr<TransitionComponent> p_transitionComponent
        );

        /// Removes a transition component.
        void RemoveTransitionComponent(TransitionComponent *p_transitionComponent);

        /// Starts a transition to another scene.
        void BeginTransition(
            const std::string &p_targetSceneName,
            std::weak_ptr<Scene> p_currentScene
        );

        void LoadTargetScene(const std::string &p_targetSceneName);

    protected:
        void StartImpl() override;

    private:
        std::vector<std::weak_ptr<TransitionComponent>> m_transitionComponents;
    };
}
