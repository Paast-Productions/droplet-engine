#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"

#include <memory>

namespace Droplet::Scene
{
    class Node;

    class PlayerBehaviour : public Behaviour
    {
    public:
        PlayerBehaviour() = default;

        std::string_view GetTypeName() override { return "PlayerBehaviour"; }

        /// @brief Sets the Node representing the player.
        /// @param p_player The player Node.
        void SetPlayer(const std::shared_ptr<Node> &p_player) { m_player = p_player; }

        /// @brief Gets the player Node, if it still exists.
        [[nodiscard]] std::shared_ptr<Node> GetPlayer() const { return m_player.lock(); }

        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const { return m_data; }

        // HEALTH

        void SetMaxHealth(float p_maxHealth) { m_data.maxHealth = p_maxHealth; }

        [[nodiscard]] float GetMaxHealth() { return m_data.maxHealth; }

        // HEAT

        void SetHeatRestistance(float p_heatRestitance) { m_data.heatResistance = p_heatRestitance; }

        [[nodiscard]] float GetHeatRestistance() { return m_data.heatResistance; }

        void IncreaseHeatLevel(float p_amount) { m_data.heatLevel += p_amount; }
        
        void SetHeatLevel(float p_heat) { m_data.heatLevel = p_heat; }

        [[nodiscard]] float GetHeatLevel() { return m_data.heatLevel; }

    protected:
        /// @brief Internal rendering function for the component's UI.
        /// Overloaded by derived components to implement their own UI rendering logic.
        void RenderUIImpl() override;

        [[nodiscard]] nlohmann::json SerializeImpl() override;

        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        std::weak_ptr<Node> m_player;
        Droplet::Game::PlayerData m_data;
    };
}