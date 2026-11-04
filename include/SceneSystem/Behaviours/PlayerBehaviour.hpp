#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"
#include <SceneSystem/Items/Item.hpp>
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

    /// @brief Identifies which player inventory an operation targets.
    enum class InventoryType
    {
        Player, ///< The inventory belonging to the player.
        Drone   ///< The inventory belonging to the player's drone.
    };

    /// @brief Behaviour responsible for managing persistent player data.
    ///
    /// Stores player-related data such as health, heat resistance, heat level,
    /// and inventories. Also maintains a weak reference to the Node representing
    /// the player in the current scene.
    ///
    /// The item registry is owned by the game and accessed through the
    /// SceneManager. SetSceneManager must have been called, and the registry
    /// must have been assigned to the SceneManager, before this behaviour
    /// is deserialized.
    ///
    /// @see PlayerData
    /// @see InventoryType
    /// @see Droplet::Game::ItemRegistry
    class PlayerBehaviour : public Behaviour
    {
    public:
        /// @brief Constructs a PlayerBehaviour with default player data.
        PlayerBehaviour() = default;

        /// @brief Gets the behaviour's registered type name.
        /// @return String identifying this behaviour type.
        [[nodiscard]] std::string_view GetTypeName() override { return "PlayerBehaviour"; }

        /// @brief Sets the Node representing the player.
        ///
        /// Stores a weak reference to the supplied Node.
        ///
        /// @param p_player Shared pointer to the player's Node.
        void SetPlayer(const std::shared_ptr<Node> &p_player) { m_player = p_player; }

        /// @brief Gets the Node representing the player.
        /// @return Shared pointer to the player's Node, or nullptr if it no longer exists.
        [[nodiscard]] std::shared_ptr<Node> GetPlayer() const { return m_player.lock(); }

        /// @brief Gets the player's persistent data.
        /// @return Constant reference to the stored PlayerData.
        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const { return m_data; }

        // HEALTH

        /// @brief Sets the maximum health of the player.
        /// @param p_maxHealth New maximum health value.
        void SetMaxHealth(float p_maxHealth) { m_data.maxHealth = p_maxHealth; }

        /// @brief Gets the maximum health of the player.
        /// @return Current maximum health value.
        [[nodiscard]] float GetMaxHealth() const { return m_data.maxHealth; }

        // HEAT

        /// @brief Sets the player's resistance to environmental heat.
        /// @param p_heatResistance New heat resistance value.
        void SetHeatResistance(float p_heatResistance) { m_data.heatResistance = p_heatResistance; }

        /// @brief Gets the player's resistance to environmental heat.
        /// @return Current heat resistance value.
        [[nodiscard]] float GetHeatResistance() const { return m_data.heatResistance; }

        /// @brief Increases the player's current heat level.
        /// @param p_amount Amount to add to the current heat level.
        void IncreaseHeatLevel(float p_amount) { m_data.heatLevel += p_amount; }

        /// @brief Sets the player's current heat level.
        /// @param p_heat New heat level value.
        void SetHeatLevel(float p_heat) { m_data.heatLevel = p_heat; }

        /// @brief Gets the player's current heat level.
        /// @return Current heat level value.
        [[nodiscard]] float GetHeatLevel() const { return m_data.heatLevel; }
        
        /// Freeze
        
        /// @brief Sets the player's resistance to environmental heat.
        /// @param p_heatResistance New heat resistance value.
        void SetFreezeResistance(float p_freezeResistance) { m_data.freezeResistance = p_freezeResistance; }

        /// @brief Gets the player's resistance to environmental heat.
        /// @return Current heat resistance value.
        [[nodiscard]] float GetFreezeResistance() const { return m_data.freezeResistance; }

        /// @brief Increases the player's current heat level.
        /// @param p_amount Amount to add to the current heat level.
        void IncreaseFreezeLevel(float p_amount) { m_data.freezeLevel += p_amount; }

        /// @brief Sets the player's current heat level.
        /// @param p_heat New heat level value.
        void SetFreezeLevel(float p_freeze) { m_data.freezeLevel = p_freeze; }

        /// @brief Gets the player's current heat level.
        /// @return Current heat level value.
        [[nodiscard]] float GetFreezeLevel() const { return m_data.freezeLevel; }

        // ITEMS

        /// @brief Gets the item registry assigned to the SceneManager.
        ///
        /// The registry is used to retrieve item definitions when performing
        /// inventory operations.
        ///
        /// @return Pointer to the item registry, or nullptr if no SceneManager
        ///         exists or no registry has been assigned.
        [[nodiscard]] const Droplet::Game::ItemRegistry *GetItemRegistry() const
        {
            const SceneManager *sceneManager = GetSceneManager();
            return sceneManager ? sceneManager->GetItemRegistry() : nullptr;
        }

        /// @brief Adds items to one of the player's inventories.
        ///
        /// Existing stacks are filled before new stacks are created.
        /// Items that cannot fit in the inventory are not added.
        ///
        /// @param p_inv Inventory to add items to.
        /// @param p_id Identifier of the item to add.
        /// @param p_count Number of items to add.
        /// @return Number of items that could not be added. Returns 0 if all items fit.
        std::uint32_t AddItem(
            InventoryType p_inv,
            Droplet::Game::ItemId p_id,
            std::uint32_t p_count);

        /// @brief Removes items from one of the player's inventories.
        ///
        /// The operation is all-or-nothing. Items are only removed if the
        /// inventory contains at least the requested amount.
        ///
        /// @param p_inv Inventory to remove items from.
        /// @param p_id Identifier of the item to remove.
        /// @param p_count Number of items to remove.
        /// @return True if all requested items were removed, otherwise false.
        bool RemoveItem(
            InventoryType p_inv,
            Droplet::Game::ItemId p_id,
            std::uint32_t p_count);

        /// @brief Counts the total number of an item in one of the player's inventories.
        ///
        /// Counts matching items across all stacks in the selected inventory.
        ///
        /// @param p_inv Inventory to search.
        /// @param p_id Identifier of the item to count.
        /// @return Total number of matching items.
        [[nodiscard]] std::uint32_t CountItem(
            InventoryType p_inv,
            Droplet::Game::ItemId p_id) const;

    protected:
        /// @brief Renders the behaviour's properties in the editor.
        ///
        /// Provides an interface for inspecting and editing player data.
        void RenderUIImpl() override;

        /// @brief Serializes the player's persistent data into JSON.
        /// @return JSON object containing the player's data.
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        /// @brief Restores the player's persistent data from JSON.
        ///
        /// Inventory item names are resolved through the item registry.
        ///
        /// @param p_behJson JSON object containing the serialized behaviour data.
        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        /// @brief Gets the inventory selected by the specified inventory type.
        /// @param p_type Inventory to retrieve.
        /// @return Mutable reference to the selected inventory.
        std::vector<Droplet::Game::ItemStack> &Inv(InventoryType p_type)
        {
            return p_type == InventoryType::Player ? m_data.playerInventory : m_data.droneInventory;
        }

        /// @brief Gets the inventory selected by the specified inventory type.
        /// @param p_type Inventory to retrieve.
        /// @return Constant reference to the selected inventory.
        const std::vector<Droplet::Game::ItemStack> &Inv(InventoryType p_type) const
        {
            return p_type == InventoryType::Player ? m_data.playerInventory : m_data.droneInventory;
        }

        std::weak_ptr<Node> m_player;     ///< Weak reference to the Node representing the player.
        Droplet::Game::PlayerData m_data; ///< Persistent data associated with the player.
    };
}