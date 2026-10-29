#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
#include <SceneSystem/Behaviour.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include "PlayerComponent.hpp"


namespace Droplet::Scene
{
    void PlayerComponent::StartImpl()
    {
        auto owner = GetOwner();
        if (!owner)
            return;

        auto scene = owner->GetScene();
        if (!scene)
            return;

        auto playerBehaviour = scene->GetBehaviour<PlayerBehaviour>();

        if (!playerBehaviour)
        {
            auto startedBehaviour = scene->StartBehaviour("PlayerBehaviour").lock();

            playerBehaviour = std::dynamic_pointer_cast<PlayerBehaviour>(startedBehaviour);
        }

        if (!playerBehaviour)
            return;

        m_playerBehaviour = playerBehaviour;
        playerBehaviour->SetPlayer(owner);
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
        return nlohmann::json::object();
    }

    void PlayerComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {
        StartImpl();
    }
}