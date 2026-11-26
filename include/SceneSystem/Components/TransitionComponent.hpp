
#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/Scene.hpp>
#include <string>
#include <string_view>
#include <memory>

namespace Droplet::Scene
{
    class TransitionBehaviour;

    class TransitionComponent : public Component
    {
    public:
        ~TransitionComponent() override;

        [[nodiscard]] std::string_view GetTypeName() const override
        {
            return "TransitionComponent";
        }

        void SetTargetScene(const std::string &p_sceneName)
        {
            m_targetSceneName = p_sceneName;
        }

        [[nodiscard]] const std::string &GetTargetScene() const
        {
            return m_targetSceneName;
        }

        [[nodiscard]] std::weak_ptr<TransitionBehaviour> GetTransitionBehaviour() const
        {
            return m_transitionBehaviour;
        }

        void BeginTransition();

    protected:
        void ConstructImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;

        [[nodiscard]] nlohmann::json SerializeImpl();
        void DeserializeImpl(const nlohmann::json p_compJson);

    private:
        std::weak_ptr<TransitionBehaviour> m_transitionBehaviour;

        std::string m_targetSceneName;
		bool m_targetSceneLoaded = false;
    };
}
