#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    class PlayerComponent : public Component
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() const override { return "PlayerComponent"; }
    };
}