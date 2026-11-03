#include <SceneSystem/Items/BaseItems.hpp>
#include <SceneSystem/Items/ItemRegistry.hpp>
#include <cassert>

namespace Droplet::Game
{
    void RegisterBaseItems(ItemRegistry &p_registry)
    {
        Items::Wood = p_registry.Register({ "resource:wood", "Wood", 16 });
        Items::Stone = p_registry.Register({ "resource:stone", "Stone", 16 });
        Items::Iron = p_registry.Register({ "resource:iron", "Iron", 16 });

        assert(Items::Wood);
        assert(Items::Stone);
        assert(Items::Iron);
    }
}