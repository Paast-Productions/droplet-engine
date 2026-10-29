#include <SceneSystem/Components/PlayerComponent.hpp>

using namespace Droplet::Scene;
{
    void PlayerComponent::StartImpl()
    {
    }

    void PlayerComponent::UpdateImpl()
    {
        // No per-frame logic is currently required.
    }


    void PlayerComponent::RenderUIImpl()
    {

    }

    nlohmann::json PlayerComponent::SerializeImpl()
    {

    }

    void PlayerComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {

    }
}