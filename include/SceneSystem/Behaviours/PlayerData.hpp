#pragma once

#include <vector>

#include <SceneSystem/Items/Item.hpp>

namespace Droplet::Game
{
    struct PlayerData
    {
        std::vector<ItemStack> droneInventory;
        std::vector<ItemStack> playerInventory;
        float maxHealth = 100;
        float heatResistance = 50;
        float heatLevel = 10;
        float pressure = 0;
    };
}