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

        void AddPodComponent(std::weak_ptr<PodComponent> podComponent);
		void RemovePodComponent(PodComponent *podComponent);

		std::weak_ptr<PodComponent> GetPodInScene(std::weak_ptr<Scene> scene) const;

    protected:
        void RenderUIImpl() override;

    private:
        std::vector<std::weak_ptr<PodComponent>> m_podComponents;
        //std::weak_ptr<Node> m_player;
    };
}