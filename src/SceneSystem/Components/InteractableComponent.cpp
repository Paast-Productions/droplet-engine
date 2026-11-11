#include "InteractableComponent.hpp"

#include <Components/ScriptComponent.hpp>
#include <Node.hpp>
#include <Time.hpp>

using namespace Droplet::Scene;

void InteractableComponent::StartImpl()
{
}

void InteractableComponent::UpdateImpl()
{
}

void InteractableComponent::RenderUIImpl()
{
    ImGui::Text("InteractableComponent");
}

void InteractableComponent::Interact(const std::shared_ptr<Node> &p_interactingNode)
{
    auto owner = GetOwner();

    if (!owner || !p_interactingNode)
    {
        return;
    }

    auto scripts = owner->GetComponents<ScriptComponent>();

    for (const auto &script : scripts)
    {
        if (!script)
        {
            continue;
        }

        ScriptSystem::Get().TriggerComponentScript(
            script.get(),
            Droplet::Time::Get().GetDeltaTime(),
            p_interactingNode
        );
    }
}