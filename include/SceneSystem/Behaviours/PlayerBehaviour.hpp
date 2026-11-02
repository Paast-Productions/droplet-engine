#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace Droplet::Game
{
    class ItemRegistry;
}

namespace Droplet::Scene
{
    class Node;

    /// @brief Which of the player's inventories an operation targets.
    enum class InventoryType
    {
        Player,
        Drone
    };

    /// @brief Behaviour responsible for managing persistent player data.
    ///
    /// Stores player-related data such as health, heat resistance, heat level,
    /// and inventory. Also maintains a weak reference to the Node representing
    /// the player in the current scene.
    ///
    /// The item registry is owned by the game and reached through the SceneManager,
    /// so SetSceneManager must have been called (and the SceneManager must have its    
    /// registry set) before this behaviour is deserialized.
    class PlayerBehaviour : public Behaviour
    {
    public:
        /// @brief Constructs a PlayerBehaviour with default player data.
        PlayerBehaviour() = default;

        /// @brief Gets the behaviour's registered type name.
        [[nodiscard]] std::string_view GetTypeName() override { return "PlayerBehaviour"; }

        /// @brief Sets the Node representing the player.
        void SetPlayer(const std::shared_ptr<Node> &p_player) { m_player = p_player; }

        /// @brief Gets the Node representing the player, if it still exists.
        [[nodiscard]] std::shared_ptr<Node> GetPlayer() const { return m_player.lock(); }

        /// @brief Gets the player's persistent data.
        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const { return m_data; }

        // HEALTH

        void SetMaxHealth(float p_maxHealth) { m_data.maxHealth = p_maxHealth; }
        [[nodiscard]] float GetMaxHealth() const { return m_data.maxHealth; }

        // HEAT

        void SetHeatResistance(float p_heatResistance) { m_data.heatResistance = p_heatResistance; }
        [[nodiscard]] float GetHeatResistance() const { return m_data.heatResistance; }

        void IncreaseHeatLevel(float p_amount) { m_data.heatLevel += p_amount; }
        void SetHeatLevel(float p_heat) { m_data.heatLevel = p_heat; }
        [[nodiscard]] float GetHeatLevel() const { return m_data.heatLevel; }

        // ITEMS

        /// @brief Gets the item registry from the SceneManager.
        /// @return The registry, or nullptr if there is no SceneManager or it has no registry yet.
        [[nodiscard]] const Droplet::Game::ItemRegistry *GetItemRegistry() const
        {
            const SceneManager *sceneManager = GetSceneManager();
            return sceneManager ? sceneManager->GetItemRegistry() : nullptr;
        }

        /// @brief Adds items to an inventory, topping up stacks first.
        /// @return How many items did NOT fit (0 = all added).
        std::uint32_t AddItem(InventoryType p_inv, Droplet::Game::ItemId p_id, std::uint32_t p_count);

        /// @brief Removes items from an inventory, all-or-nothing.
        /// @return True if the full amount was removed.
        bool RemoveItem(InventoryType p_inv, Droplet::Game::ItemId p_id, std::uint32_t p_count);

        /// @brief Total number of an item across all stacks of an inventory.
        [[nodiscard]] std::uint32_t CountItem(InventoryType p_inv, Droplet::Game::ItemId p_id) const;

    protected:
        /// @brief Renders the behaviour's properties in the editor.
        void RenderUIImpl() override;

        /// @brief Serializes the player's persistent data.
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        /// @brief Restores the player's persistent data from JSON.
        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        std::vector<Droplet::Game::ItemStack> &Inv(InventoryType p_type)
        {
            return p_type == InventoryType::Player ? m_data.playerInventory : m_data.droneInventory;
        }

        const std::vector<Droplet::Game::ItemStack> &Inv(InventoryType p_type) const
        {
            return p_type == InventoryType::Player ? m_data.playerInventory : m_data.droneInventory;
        }

        std::weak_ptr<Node> m_player;      /// Weak reference to the Node representing the player.
        Droplet::Game::PlayerData m_data;  /// Persistent data associated with the player.
    };
}