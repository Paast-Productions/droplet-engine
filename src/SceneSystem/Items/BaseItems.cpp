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
        Items::Metal = p_registry.Register({ "resource:metal", "Metal", 16 });
        Items::Eel = p_registry.Register({ "resource:eel", "Eel", 16 });
        Items::Copper = p_registry.Register({ "resource:copper", "Copper", 16 });
        Items::Tin = p_registry.Register({ "resource:tin", "Tin", 16 });
        Items::Bronze = p_registry.Register({ "resource:bronze", "Bronze", 16 });
        Items::Lead = p_registry.Register({ "resource:lead", "Lead", 16 });

        assert(Items::Wood);
        assert(Items::Stone);
        assert(Items::Iron);
        assert(Items::Metal);
        assert(Items::Eel);
        assert(Items::Copper);
        assert(Items::Tin);
        assert(Items::Bronze);
        assert(Items::Lead);
    }
}