#pragma once

#include <cstdint>
#include <string>

namespace Droplet::Game
{
    struct ItemDefinition
    {
        std::string id;
        std::string name;
        std::string description;

        std::uint32_t maxStackSize = 99;
    };
}