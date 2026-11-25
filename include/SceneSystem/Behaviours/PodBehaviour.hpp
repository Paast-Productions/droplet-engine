#pragma once
#include <SceneSystem/Behaviour.hpp>
#include <string_view>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
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
		[[nodiscard]] std::weak_ptr<PodComponent> GetPodInScene(const std::string &p_sceneName) const;

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

        struct PendingTransition
        {
            std::string targetSceneName;
            std::weak_ptr<Scene> currentScene;
        };

        std::optional<PendingTransition> m_pendingTransition;
        // Set when BoatScene was requested by this behaviour, so the pod inventory is transferred once it is ready
        bool m_transferInventoryOnBoatLoad = false;
        // Shared so that load callbacks remain valid if this behaviour is destroyed first
        std::shared_ptr<std::unordered_set<std::string>> m_loadingScenes = std::make_shared<std::unordered_set<std::string>>();

        void LoadPodScene();
        void RequestSceneLoad(const std::string &p_sceneName);
        void UpdatePendingTransition();
        void StartTransition(std::shared_ptr<Scene> p_targetScene);
    };
}