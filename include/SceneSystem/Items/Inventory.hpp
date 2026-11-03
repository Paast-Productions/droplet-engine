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

    /// @brief Returns the total number of an item across all inventory stacks.
    ///
    /// @param p_inventory Inventory to search.
    /// @param p_id Identifier of the item to count.
    /// @return Total number of matching items.
    std::uint32_t CountItem(
        const std::vector<ItemStack> &p_inventory,
        ItemId p_id);

    /// @brief Adds items to an inventory, filling existing stacks before creating new ones.
    ///
    /// Existing stacks of the specified item are filled up to their maximum stack size.
    /// New stacks are created if space is available.
    ///
    /// @param p_registry Registry used to retrieve the item's definition.
    /// @param p_inventory Inventory to add items to.
    /// @param p_id Identifier of the item to add.
    /// @param p_count Number of items to add.
    /// @param p_maxSlots Maximum number of stacks allowed in the inventory.
    ///                   Defaults to an unlimited number of stacks.
    /// @return Number of items that could not be added. Returns 0 if all items fit.
    std::uint32_t AddItem(
        const ItemRegistry &p_registry,
        std::vector<ItemStack> &p_inventory,
        ItemId p_id,
        std::uint32_t p_count,
        std::size_t p_maxSlots = std::numeric_limits<std::size_t>::max());

    /// @brief Removes a specified number of items from an inventory.
    ///
    /// The operation is all-or-nothing. Items are only removed if the inventory
    /// contains at least the requested amount. Empty stacks are removed afterwards.
    ///
    /// @param p_inventory Inventory to remove items from.
    /// @param p_id Identifier of the item to remove.
    /// @param p_count Number of items to remove.
    /// @return True if the items were successfully removed, otherwise false.
    bool RemoveItem(
        std::vector<ItemStack> &p_inventory,
        ItemId p_id,
        std::uint32_t p_count);

    /// @brief Represents an inventory stack in a save-compatible format.
    ///
    /// Uses the item's stable name instead of its runtime ItemId, allowing
    /// inventories to be restored even when item registration order changes.
    struct SavedStack
    {
        std::string name;           ///< Stable identifier of the item.
        std::uint32_t count = 0;    ///< Number of items in the stack.
    };

    /// @brief Converts inventory stacks into a save-compatible representation.
    ///
    /// Item identifiers are converted to their registered names.
    /// Stacks containing unknown item identifiers are skipped.
    ///
    /// @param p_registry Registry used to resolve item identifiers to names.
    /// @param p_inventory Inventory to convert.
    /// @return Vector containing the converted inventory stacks.
    ///
    /// @see FromSaved
    std::vector<SavedStack> ToSaved(
        const ItemRegistry &p_registry,
        const std::vector<ItemStack> &p_inventory);

    /// @brief Restores inventory stacks from their save-compatible representation.
    ///
    /// Saved item names are resolved to runtime ItemIds using the registry.
    /// Unknown item names are skipped, allowing saves to remain loadable when
    /// items have been removed or their corresponding mods are unavailable.
    ///
    /// @param p_registry Registry used to resolve item names.
    /// @param p_saved Saved inventory stacks to restore.
    /// @param p_skipped Optional pointer to a counter incremented for each unknown item.
    /// @return Vector containing the successfully restored inventory stacks.
    ///
    /// @see ToSaved
    std::vector<ItemStack> FromSaved(
        const ItemRegistry &p_registry,
        const std::vector<SavedStack> &p_saved,
        std::size_t *p_skipped = nullptr);
}