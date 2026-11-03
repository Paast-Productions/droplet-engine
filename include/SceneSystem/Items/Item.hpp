#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace Droplet::Game
{
    /// @brief Runtime identifier for an item type.
    ///
    /// ItemId wraps a uint32_t to prevent accidental mixing of item identifiers
    /// with unrelated integer values such as counts or indices.
    ///
    /// The value 0 is reserved as an invalid identifier.
    ///
    /// @note ItemIds are only valid during the current runtime session.
    ///       Use the item's stable name when saving or loading item references.
    struct ItemId
    {
        std::uint32_t value = 0; ///< Internal runtime identifier. 0 represents an invalid ID.

        /// @brief Checks whether the identifier is valid.
        /// @return True if the identifier is not zero, otherwise false.
        constexpr bool IsValid() const { return value != 0; }

        /// @brief Explicitly converts the identifier to a boolean.
        /// @return True if the identifier is valid, otherwise false.
        constexpr explicit operator bool() const { return value != 0; }

        /// @brief Compares two item identifiers for equality.
        /// @param p_a First identifier.
        /// @param p_b Second identifier.
        /// @return True if both identifiers have the same value.
        friend constexpr bool operator==(ItemId p_a, ItemId p_b)
        {
            return p_a.value == p_b.value;
        }

        /// @brief Compares two item identifiers for inequality.
        /// @param p_a First identifier.
        /// @param p_b Second identifier.
        /// @return True if the identifiers have different values.
        friend constexpr bool operator!=(ItemId p_a, ItemId p_b)
        {
            return p_a.value != p_b.value;
        }

        /// @brief Compares two item identifiers by their underlying values.
        /// @param p_a First identifier.
        /// @param p_b Second identifier.
        /// @return True if the value of p_a is less than the value of p_b.
        friend constexpr bool operator<(ItemId p_a, ItemId p_b)
        {
            return p_a.value < p_b.value;
        }
    };

    /// @brief Invalid item identifier.
    ///
    /// Represents an unassigned or invalid item reference.
    inline constexpr ItemId InvalidItemId{};

    /// @brief Definition describing an item type.
    ///
    /// Contains the shared properties of an item type rather than
    /// the properties of an individual item instance or stack.
    ///
    /// @see ItemId
    /// @see ItemStack
    struct ItemDef
    {
        std::string name;               ///< Stable identifier used for registration and saving.
        std::string displayName;        ///< User-facing name displayed in the UI.
        std::uint32_t maxStack = 64;    ///< Maximum number of items allowed in one stack. 1 means non-stackable.
    };

    /// @brief Represents a stack of items of the same type.
    ///
    /// Stores an item identifier and the number of items in the stack.
    /// Additional item properties are retrieved through the ItemRegistry.
    ///
    /// @see ItemDef
    /// @see ItemRegistry
    struct ItemStack
    {
        ItemId item;                    ///< Identifier of the item type.
        std::uint32_t count = 0;        ///< Number of items in the stack.
    };
}

/// @brief Provides a hash implementation for ItemId.
///
/// Allows ItemId to be used as a key in unordered associative containers.
namespace std
{
    template <>
    struct hash<Droplet::Game::ItemId>
    {
        /// @brief Calculates the hash value of an ItemId.
        /// @param p_id Item identifier to hash.
        /// @return Hash value of the underlying identifier.
        size_t operator()(Droplet::Game::ItemId p_id) const noexcept
        {
            return std::hash<std::uint32_t>{}(p_id.value);
        }
    };
}