#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"

#include <memory>

namespace Droplet::Scene
{
    class Node;

    /// @brief Behaviour responsible for managing persistent player data.
    ///
    /// Stores player-related data such as health, heat resistance, heat level,
    /// and inventory. Also maintains a weak reference to the Node representing
    /// the player in the current scene.
    class PlayerBehaviour : public Behaviour
    {
    public:
        /// @brief Constructs a PlayerBehaviour with default player data.
        PlayerBehaviour() = default;

        /// @brief Gets the behaviour's registered type name.
        /// @return The type name of the behaviour.
        [[nodiscard]] std::string_view GetTypeName() override { return "PlayerBehaviour"; }

        /// @brief Sets the Node representing the player.
        /// @param p_player Shared pointer to the player's Node.
        void SetPlayer(const std::shared_ptr<Node> &p_player) { m_player = p_player; }

        /// @brief Gets the Node representing the player, if it still exists.
        /// @return Shared pointer to the player Node, or nullptr if it no longer exists.
        [[nodiscard]] std::shared_ptr<Node> GetPlayer() const { return m_player.lock(); }

        /// @brief Gets the player's persistent data.
        /// @return Constant reference to the PlayerData instance.
        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const { return m_data; }

        // HEALTH

        /// @brief Sets the player's maximum health.
        /// @param p_maxHealth The new maximum health value.
        void SetMaxHealth(float p_maxHealth) { m_data.maxHealth = p_maxHealth; }

        /// @brief Gets the player's maximum health.
        /// @return The player's maximum health value.
        [[nodiscard]] float GetMaxHealth() const { return m_data.maxHealth; }

        // HEAT

        /// @brief Sets the player's heat resistance.
        /// @param p_heatResistance The new heat resistance value.
        void SetHeatResistance(float p_heatResistance) { m_data.heatResistance = p_heatResistance; }

        /// @brief Gets the player's heat resistance.
        /// @return The player's heat resistance value.
        [[nodiscard]] float GetHeatResistance() const { return m_data.heatResistance; }

        /// @brief Increases the player's heat level.
        /// @param p_amount The amount to add to the current heat level.
        void IncreaseHeatLevel(float p_amount) { m_data.heatLevel += p_amount; }

        /// @brief Sets the player's heat level.
        /// @param p_heat The new heat level value.
        void SetHeatLevel(float p_heat) { m_data.heatLevel = p_heat; }

        /// @brief Gets the player's current heat level.
        /// @return The player's current heat level.
        [[nodiscard]] float GetHeatLevel() const { return m_data.heatLevel; }

    protected:
        /// @brief Renders the behaviour's properties in the editor.
        void RenderUIImpl() override;

        /// @brief Serializes the player's persistent data.
        /// @return A JSON object containing the serialized player data.
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        /// @brief Restores the player's persistent data from JSON.
        /// @param p_behJson The JSON object containing the serialized player data.
        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        std::weak_ptr<Node> m_player; /// Weak reference to the Node representing the player.
        Droplet::Game::PlayerData m_data; /// Persistent data associated with the player.
    };
}