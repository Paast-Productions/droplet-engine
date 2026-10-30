#pragma once

#include "ItemDefinition.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Droplet::Game
{
    /// @brief Stores and provides access to item definitions.
    ///
    /// Item definitions are loaded from a JSON file and indexed by
    /// their unique IDs.
    class ItemDatabase
    {
    public:
        /// @brief Loads item definitions from a JSON file.
        /// @param p_filePath Path to the JSON file.
        /// @return True if loading succeeded, otherwise false.
        bool LoadFromFile(const std::string &p_filePath);

        /// @brief Gets an item definition by its ID.
        /// @param p_itemId Unique identifier of the item.
        /// @return Pointer to the item definition, or nullptr if not found.
        [[nodiscard]] const ItemDefinition *GetItem(
            std::string_view p_itemId) const;

        /// @brief Checks whether an item exists in the database.
        /// @param p_itemId Unique identifier of the item.
        /// @return True if the item exists, otherwise false.
        [[nodiscard]] bool Contains(std::string_view p_itemId) const;

        /// @brief Gets the number of registered item definitions.
        /// @return Number of registered items.
        [[nodiscard]] std::size_t GetItemCount() const;

        /// @brief Gets all registered item definitions.
        /// @return Constant reference to the item definitions.
        [[nodiscard]] const std::unordered_map<std::string, ItemDefinition> &GetItems() const
        {
            return m_items;
        }

        /// @brief Removes all registered item definitions.
        void Clear();

    private:
        std::unordered_map<std::string, ItemDefinition> m_items;
    };
}