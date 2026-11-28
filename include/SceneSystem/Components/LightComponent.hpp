#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    class LightComponent : public Component
    {
    public:
        DamageComponent() = default;

        std::string_view GetTypeName() const override { return "LightComponent"; }

        /// @brief Gets the amount of damage per second, only used in Lua scripts
        /// @return The amount of damage
        [[nodiscard]] float GetDamageLua() const { return m_damage; }

    protected:
        void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;

        void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override;

    private:
        
        glm::vec3 m : color{ 1.0f };
        float intensity{ 5.0f };
        float m_range{ 20.0f };

    };
}
