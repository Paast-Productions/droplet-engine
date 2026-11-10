#include <SceneSystem/Items/BaseItems.hpp>
#include <SceneSystem/Items/ItemRegistry.hpp>
#include <cassert>

namespace Droplet::Game
{
    void RegisterBaseItems(ItemRegistry &p_registry)
    {
        Items::Wood = p_registry.Register({ "resource:wood", "Wood", 128 });
        Items::Stone = p_registry.Register({ "resource:stone", "Stone", 128 });
        Items::Iron = p_registry.Register({ "resource:iron", "Iron", 128 });
		Items::BrokenEnginePart = p_registry.Register({ "upgrade:broken_engine_part", "Broken Engine Part", 1 });
		Items::RepairedEnginePart = p_registry.Register({ "upgrade:repaired_engine_part", "Repaired Engine Part", 1 });

        assert(Items::Wood);
        assert(Items::Stone);
        assert(Items::Iron);
		assert(Items::BrokenEnginePart);
		assert(Items::RepairedEnginePart);
    }
}