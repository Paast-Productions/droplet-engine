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
    /// @brief Owns every ItemDef and maps name <-> id.
    ///
    /// Ids are indices (+1) into storage and are only ever appended, so an ItemId stays
    /// valid for the whole run, even if more items are registered later (mods, hot reload,
    /// server-sent items).
    class ItemRegistry
    {
    public:
        /// @brief Adds a new item type. If the name is already registered, returns the
        /// existing id and leaves its definition untouched.
        /// @return The item's id, or InvalidItemId for an empty name.
        ItemId Register(ItemDef def);

        /// @brief Like Register, but if the name already exists its data is replaced in
        /// place (same id). Use for hot reload / data patches.
        ItemId RegisterOrUpdate(ItemDef def);

        /// @brief name -> id. Returns InvalidItemId if unknown.
        /// Hash lookup: do this at load time and cache the result, not every frame.
        ItemId Find(std::string_view name) const;

        /// @brief id -> definition. nullptr for invalid/unknown ids. O(1).
        /// The pointer stays valid across later Register calls (deque), but prefer
        /// holding the ItemId and calling Get when you need it.
        const ItemDef *Get(ItemId id) const
        {
            if (!id.IsValid() || id.value > m_defs.size())
                return nullptr;
            return &m_defs[id.value - 1];
        }

        /// @brief id -> stable name, for writing saves. Empty string for invalid ids.
        const std::string &NameOf(ItemId id) const
        {
            static const std::string empty;
            const ItemDef *def = Get(id);
            return def ? def->name : empty;
        }

        std::size_t Size() const { return m_defs.size(); }

        /// @brief Calls f(ItemId, const ItemDef&) for every registered item.
        template <typename F>
        void ForEach(F &&f) const
        {
            for (std::size_t i = 0; i < m_defs.size(); ++i)
                f(ItemId{ static_cast<std::uint32_t>(i + 1) }, m_defs[i]);
        }

    private:
        ItemId Append(ItemDef &&def);

        std::deque<ItemDef> m_defs; /// index = id.value - 1
        std::unordered_map<std::string_view, ItemId> m_byName;
    };
}