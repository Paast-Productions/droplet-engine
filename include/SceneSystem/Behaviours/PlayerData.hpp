#pragma once

#include <vector>

#include <SceneSystem/Items/Item.hpp>

namespace Droplet::Game
{
    /// @brief Stores the player's runtime gameplay data.
    ///
    /// Contains player statistics and inventory data, including the inventory
    /// belonging to the player and their drone.
    struct PlayerData
    {
        std::vector<ItemStack> droneInventory;  ///< Items stored in the drone's inventory.
        std::vector<ItemStack> playerInventory; ///< Items stored in the player's inventory.

        std::size_t maxDroneInventorySlots = 20;
        std::size_t maxPlayerInventorySlots = 20;

        float maxHealth = 100;      ///< Maximum health of the player.
        float heatResistance = 50; ///< Resistance to environmental heat.
        float heatLevel = 10;       ///< Current heat level of the player.
        float pressure = 0;         ///< Current environmental pressure affecting the player.
        float freezeResistance = 50; ///< Resistance to environmental freeze.
        float freezeLevel = 0;      ///< Current freeze level of the player.
        float timeSinceFreeze = 0;
    };
}