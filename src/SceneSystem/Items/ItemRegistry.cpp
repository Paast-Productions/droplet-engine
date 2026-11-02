#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "SceneSystem/Items/Item.hpp"
#include "SceneSystem/Items/ItemRegistry.hpp"

namespace Droplet::Game
{
	ItemId ItemRegistry::Register(ItemDef def)
	{
		if (def.name.empty())
		{
			assert(false && "ItemDef needs a name");
			return InvalidItemId;
		}
		if (ItemId existing = Find(def.name))
			return existing;
		return Append(std::move(def));
	}

    ItemId ItemRegistry::RegisterOrUpdate(ItemDef def)
    {
        if (def.name.empty())
        {
            assert(false && "ItemDef needs a name");
            return InvalidItemId;
        }

        if (ItemId existing = Find(def.name))
        {
            ItemDef &d = m_defs[existing.value - 1];
            // Do not touch d.name: m_byName holds string_views into it.
            d.displayName = std::move(def.displayName);
            d.maxStack = def.maxStack;
            return existing;
        }

        return Append(std::move(def));
    }

    ItemId ItemRegistry::Find(std::string_view name) const
    {
        auto it = m_byName.find(name);
        return it != m_byName.end() ? it->second : InvalidItemId;
    }

    ItemId ItemRegistry::Append(ItemDef &&def)
    {
        m_defs.push_back(std::move(def));
        ItemId id{ static_cast<std::uint32_t>(m_defs.size()) };
        // The key views the name stored inside the deque element. Deque elements never
        // move on push_back, so the view stays valid. Never modify a registered name.
        m_byName.emplace(std::string_view(m_defs.back().name), id);
        return id;
    }
}
