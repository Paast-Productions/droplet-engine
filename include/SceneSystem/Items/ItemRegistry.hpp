#pragma once

#include <cassert>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <SceneSystem/Items/Item.hpp>

namespace Droplet::Game
{
    /// @brief Manages item definitions and maps stable names to runtime identifiers.
    ///
    /// Owns all registered ItemDef instances and provides lookup in both directions.
    ///
    /// ItemIds are assigned as indices (+1) into the internal definition storage.
    /// Definitions are only appended, ensuring that existing ItemIds remain valid
    /// throughout the runtime session.
    ///
    /// @note ItemIds are not stable between application runs. Use item names when
    ///       storing item references in save files.
    ///
    /// @see ItemId
    /// @see ItemDef
    class ItemRegistry
    {
    public:
        /// @brief Registers a new item definition.
        ///
        /// If an item with the same name is already registered, its existing
        /// identifier is returned and its definition remains unchanged.
        ///
        /// @param p_def Definition of the item to register.
        /// @return Runtime identifier of the registered item, or InvalidItemId
        ///         if the item name is empty.
        ItemId Register(ItemDef p_def);

        /// @brief Registers an item or updates its existing definition.
        ///
        /// If an item with the same name already exists, its definition is
        /// updated while preserving its existing ItemId.
        ///
        /// The stable name is not modified during an update.
        ///
        /// @param p_def Definition of the item to register or update.
        /// @return Runtime identifier of the registered or updated item,
        ///         or InvalidItemId if the item name is empty.
        ItemId RegisterOrUpdate(ItemDef p_def);

        /// @brief Finds an item identifier by its stable name.
        ///
        /// Performs a hash lookup in the registry.
        ///
        /// @param p_name Stable name of the item to find.
        /// @return Runtime identifier of the item, or InvalidItemId if unknown.
        ///
        /// @note Prefer resolving item names during loading and caching their
        ///       ItemIds instead of performing lookups every frame.
        ItemId Find(std::string_view p_name) const;

        /// @brief Retrieves an item definition by its runtime identifier.
        ///
        /// @param p_id Identifier of the item to retrieve.
        /// @return Pointer to the item definition, or nullptr if the identifier
        ///         is invalid or does not exist.
        ///
        /// @note The returned pointer remains valid across subsequent Register
        ///       calls because definitions are stored in a std::deque.
        const ItemDef *Get(ItemId p_id) const
        {
            if (!p_id.IsValid() || p_id.value > m_defs.size())
                return nullptr;

            return &m_defs[p_id.value - 1];
        }

        /// @brief Retrieves the stable name of an item by its identifier.
        ///
        /// @param p_id Identifier of the item.
        /// @return Reference to the item's stable name, or an empty string
        ///         if the identifier is invalid or unknown.
        const std::string &NameOf(ItemId p_id) const
        {
            static const std::string empty;
            const ItemDef *def = Get(p_id);

            return def ? def->name : empty;
        }

        /// @brief Returns the number of registered item definitions.
        /// @return Number of registered items.
        std::size_t Size() const
        {
            return m_defs.size();
        }

        /// @brief Executes a function for every registered item.
        ///
        /// The callback receives the item's runtime identifier and a constant
        /// reference to its definition.
        ///
        /// @tparam F Callable type accepting (ItemId, const ItemDef&).
        /// @param p_f Function or callable object to execute for each item.
        template <typename F>
        void ForEach(F &&p_f) const
        {
            for (std::size_t i = 0; i < m_defs.size(); ++i)
                p_f(ItemId{ static_cast<std::uint32_t>(i + 1) }, m_defs[i]);
        }

    private:
        /// @brief Appends an item definition to the registry.
        ///
        /// Assigns a new runtime identifier and adds the item's name to the
        /// name lookup map.
        ///
        /// @param p_def Definition to append.
        /// @return Newly assigned runtime identifier.
        ItemId Append(ItemDef &&p_def);

        std::deque<ItemDef> m_defs; ///< Item definitions indexed by ItemId value minus one.
        std::unordered_map<std::string_view, ItemId> m_byName; ///< Maps stable item names to runtime identifiers.
    };
}