#pragma once

#include <SceneSystem/Items/Item.hpp>

namespace Droplet::Game
{
    class ItemRegistry;

    struct Items
    {
        static inline ItemId Wood = InvalidItemId;
        static inline ItemId Stone = InvalidItemId;
        static inline ItemId Iron = InvalidItemId;
    };

    /// Registers the built-in items and fills the Items:: cache. Call once at startup.
    void RegisterBaseItems(ItemRegistry &p_registry);
}