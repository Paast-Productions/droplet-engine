#pragma once
#include <SceneSystem/Behaviour.hpp>
#include <string_view>
#include <memory>
#include <vector>

namespace Droplet::Scene
{
    class Scene;
    class Node;
    class PodComponent;

    class PodBehaviour : public Behaviour
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() override { return "PodBehaviour"; }

        [[nodiscard]] bool IsSingleton() override { return true; }

        void AddPodComponent(std::weak_ptr<PodComponent> p_podComponent);
		void RemovePodComponent(PodComponent *p_podComponent);

		std::weak_ptr<PodComponent> GetPodInScene(std::weak_ptr<Scene> p_scene) const;
		std::weak_ptr<PodComponent> GetPodInScene(const std::string &p_sceneName);

		void BeginTransition(const std::string &p_targetSceneName, std::weak_ptr<Scene> p_currentScene);

    protected:
        
		void StartImpl() override;
		void PreUpdateImpl() override;
        void RenderUIImpl() override;

    private:
        std::vector<std::weak_ptr<PodComponent>> m_podComponents;

        std::weak_ptr<Scene> m_podScene;
        std::weak_ptr<Scene> m_targetScene;
        std::weak_ptr<Scene> m_pendingUnloadScene;

		float m_transitionTime = 3.0f;
		float m_transitionTimer = 0.0f;

        void LoadPodScene();
    };
}