#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace Droplet::Game
{
    /// @brief Runtime handle for an item type. 0 is reserved as "invalid".
    ///
    /// A tiny struct instead of a bare uint32_t so a count or index can't be passed
    /// where an item id is expected.
    ///
    /// NOT stable between runs: never write value to disk, write the item's name.
    struct ItemId
    {
        std::uint32_t value = 0;

        constexpr bool IsValid() const { return value != 0; }
        constexpr explicit operator bool() const { return value != 0; }

        friend constexpr bool operator==(ItemId a, ItemId b) { return a.value == b.value; }
        friend constexpr bool operator!=(ItemId a, ItemId b) { return a.value != b.value; }
        friend constexpr bool operator<(ItemId a, ItemId b) { return a.value < b.value; }
    };

    inline constexpr ItemId InvalidItemId{};

    /// @brief Everything that describes an item *type* (not an instance/stack).
    struct ItemDef
    {
        std::string name;               /// Stable key. Used in saves/JSON.
        std::string displayName;        /// Shown in UI.
        std::uint32_t maxStack = 64;    /// 1 = not stackable.
    };

    /// @brief A number of items of one type. Details come from ItemRegistry::Get(item).
    struct ItemStack
    {
        ItemId item;
        std::uint32_t count = 0;
    };
}

namespace std
{
    template <>
    struct hash<Droplet::Game::ItemId>
    {
        size_t operator()(Droplet::Game::ItemId id) const noexcept
        {
            return std::hash<std::uint32_t>{}(id.value);
        }
    };
}