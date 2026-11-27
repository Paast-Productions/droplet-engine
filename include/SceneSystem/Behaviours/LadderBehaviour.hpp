
#pragma once

#include <SceneSystem/Behaviour.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Droplet::Scene
{
    class Scene;
    class LadderComponent;

    class LadderBehaviour : public Behaviour
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() override
        {
            return "LadderBehaviour";
        }

        bool IsSingleton() override
        {
            return true;
        }

        /// Registers a ladder component.
        void AddLadderComponent(
            std::weak_ptr<LadderComponent> p_ladderComponent
        );

        /// Removes a ladder component.
        void RemoveLadderComponent(LadderComponent *p_ladderComponent);

        /// Starts a transition to another scene.
        void BeginTransition(
            const std::string &p_targetSceneName,
            std::weak_ptr<Scene> p_currentScene
        );

        void LoadTargetScene(const std::string &p_targetSceneName);

    protected:
        void StartImpl() override;

    private:
        std::vector<std::weak_ptr<LadderComponent>> m_ladderComponents;
    };
}
