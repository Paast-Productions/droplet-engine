#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    class DamageComponent : public Component
    {
    public:
        DamageComponent() = default;
        
        std::string_view GetTypeName() const override { return "DamageComponent"; }
        
        /// @brief Gets the amount of damage per second, only used in Lua scripts
        /// @return The amount of damage
        [[nodiscard]] float GetDamageLua() const { return m_damage; }
        
    protected:
        void RenderUIImpl() override;
        
        nlohmann::json SerializeImpl() override;
        
        void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override;
        
    private:
        float m_damage{ 1.0f }; // Amount of damage per second
        
    };
}
