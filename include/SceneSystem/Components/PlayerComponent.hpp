#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    class PlayerComponent : public Component
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() const override { return "PlayerComponent"; }

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;
        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(const nlohmann::json p_compJson) override;

    };
}