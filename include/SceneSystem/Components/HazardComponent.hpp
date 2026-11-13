#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    class HazardComponent : public Component
    {
    public:
        HazardComponent() = default;
        
        std::string_view GetTypeName() const override { return "HazardComponent"; }
        
        /// @brief Gets the amount of damage per second
        /// @return The amount of damage
        [[nodiscard]] float GetDamage() const { return m_damage; }
        
        /// @brief Gets the direction vector
        /// @return The vector of direction
        [[nodiscard]] glm::vec3 GetDirection();
        
        void SetDamage(float p_damage) { m_damage = p_damage; }
        
        /// @brief Set the direction
        /// @param p_direction The direction vector
        void SetDirection(glm::vec3 p_direction) { m_direction = glm::normalize(p_direction); }
        
    protected:
        void RenderUIImpl() override;
        
        nlohmann::json SerializeImpl() override;
        
        void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override;
        
    private:
        float m_damage{ 1.0f }; // Amount of damage per second
        glm::vec3 m_direction{ glm::vec3(0.0f, 0.0f, 1.0f) };
        
    };
}
