#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
    /// @brief Component responsible for managing an node's health.
    ///
    /// Provides functionality for tracking health, applying damage,
    /// healing, and configuring maximum health.
    class HealthComponent : public Component
    {
    public:
        /// @brief Gets the component's registered type name.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override
        {
            return "HealthComponent";
        }

        /// @brief Gets the node's current health.
        /// @return The current health value.
        [[nodiscard]] float GetHealth() const { return m_health; }

        /// @brief Gets the node's maximum health.
        /// @return The maximum health value.
        [[nodiscard]] float GetMaxHealth() const { return m_maxHealth; }

        /// @brief Sets the node's current health.
        /// @param p_health The new health value.
        void SetHealth(float p_health) { m_health = p_health; }

        /// @brief Sets the node's maximum health.
        /// @param p_maxHealth The new maximum health value.
        void SetMaxHealth(float p_maxHealth) { m_maxHealth = p_maxHealth; }

        /// @brief Applies damage to the node.
        /// @param p_amount The amount of damage to apply.
        void TakeDamage(float p_amount);

        /// @brief Restores health to the node.
        /// @param p_amount The amount of health to restore.
        void Heal(float p_amount);

    protected:
        /// @brief Initializes the component when its owner starts.
        void StartImpl() override;

        /// @brief Updates the component.
        void UpdateImpl() override;

        /// @brief Renders the component's properties in the editor.
        void RenderUIImpl() override;

        /// @brief Serializes the component's health data.
        /// @return A JSON object containing the component's data.
        nlohmann::json SerializeImpl() override;

        /// @brief Restores the component's health data from JSON.
        /// @param p_compJson The JSON object containing the serialized data.
        void DeserializeImpl(const nlohmann::json p_compJson) override;

    private:
        float m_health = 100.0f;    /// current health.
        float m_maxHealth = 100.0f; /// maximum health.
    };
}