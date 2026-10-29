#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Droplet::Game
{
    struct ItemStack
    {
        std::string itemId;
        std::uint32_t count = 0;
    };

    struct PlayerData
    {
        std::vector<ItemStack> inventory;
	    float maxHealth = 100;
        float heatResistance = 50;
    };
}
