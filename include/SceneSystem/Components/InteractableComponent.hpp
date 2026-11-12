#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    /// @brief Allows a node to be interacted with, invoking its attached Lua script.
    class InteractableComponent : public Component
    {
    public:
        InteractableComponent() = default;

        std::string_view GetTypeName() const override { return "InteractableComponent"; }

        /// @brief Invokes the OnTrigger function in the owner's script.
        /// @param p_interactingNode The node interacting with this node.
        void Interact(const std::shared_ptr<Node> &p_interactingNode);

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;
    };
}