#include <SceneSystem/Items/BaseItems.hpp>
#include <SceneSystem/Items/ItemRegistry.hpp>
#include <cassert>

namespace Droplet::Game
{
    void RegisterBaseItems(ItemRegistry &reg)
    {
        Items::Wood = reg.Register({ "droplet:wood", "Wood", 16 });
        Items::Stone = reg.Register({ "droplet:stone", "Stone", 16 });
        Items::Iron = reg.Register({ "droplet:iron", "Iron", 16 });

        assert(Items::Wood);
        assert(Items::Stone);
        assert(Items::Iron);
    }
}