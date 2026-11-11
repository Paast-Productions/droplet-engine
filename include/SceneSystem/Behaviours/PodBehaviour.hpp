#pragma once
#include <SceneSystem/Behaviour.hpp>
#include <string_view>
#include <memory>

namespace Droplet::Scene
{
    class Node;
    class PodComponent;

    class PodBehaviour : public Behaviour
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() override { return "PodBehaviour"; }

		void SetPodComponent(std::shared_ptr<PodComponent> podComponent) { m_podComponent = podComponent; }

    protected:
        void RenderUIImpl() override;

    private:
        std::weak_ptr<PodComponent> m_podComponent;
        std::weak_ptr<Node> m_player;
    };
}