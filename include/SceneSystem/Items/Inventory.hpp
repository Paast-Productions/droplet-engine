#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <SceneSystem/Items/Item.hpp>

namespace Droplet::Game
{
    class ItemRegistry;

    /// @brief Total number of an item across all stacks.
    std::uint32_t CountItem(const std::vector<ItemStack> &inv, ItemId id);

    /// @brief Adds items, topping up existing stacks first, then opening new ones.
    /// @param maxSlots Optional cap on the number of stacks.
    /// @return How many items did NOT fit (0 = everything was added).
    std::uint32_t AddItem(const ItemRegistry &reg,
        std::vector<ItemStack> &inv,
        ItemId id,
        std::uint32_t count,
        std::size_t maxSlots = std::numeric_limits<std::size_t>::max());

    /// @brief All-or-nothing: removes `count` items only if the inventory has at least that many.
    bool RemoveItem(std::vector<ItemStack> &inv, ItemId id, std::uint32_t count);

    struct SavedStack
    {
        std::string name;
        std::uint32_t count = 0;
    };

    /// @brief Converts stacks to name-based form for saving. Stacks with unknown ids are dropped.
    std::vector<SavedStack> ToSaved(const ItemRegistry &reg, const std::vector<ItemStack> &inv);

    /// @brief Converts saved names back to ids. Unknown names (item removed, mod missing)
    /// are skipped; if `skipped` is given it is incremented for each one so you can log it.
    std::vector<ItemStack> FromSaved(const ItemRegistry &reg,
        const std::vector<SavedStack> &saved,
        std::size_t *skipped = nullptr);
}