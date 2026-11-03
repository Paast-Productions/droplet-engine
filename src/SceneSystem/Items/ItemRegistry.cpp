#include <SceneSystem/Items/ItemRegistry.hpp>

#include <utility>

namespace Droplet::Game
{
    ItemId ItemRegistry::Register(ItemDef p_def)
    {
        if (p_def.name.empty())
        {
            assert(false && "ItemDef needs a name");
            return InvalidItemId;
        }

        if (ItemId existing = Find(p_def.name))
            return existing;

        return Append(std::move(p_def));
    }

    ItemId ItemRegistry::RegisterOrUpdate(ItemDef p_def)
    {
        if (p_def.name.empty())
        {
            assert(false && "ItemDef needs a name");
            return InvalidItemId;
        }

        if (ItemId existing = Find(p_def.name))
        {
            ItemDef &def = m_defs[existing.value - 1];

            // Do not touch def.name: m_byName holds string_views into it.
            def.displayName = std::move(p_def.displayName);
            def.maxStack = p_def.maxStack;

            return existing;
        }

        return Append(std::move(p_def));
    }

    ItemId ItemRegistry::Find(std::string_view p_name) const
    {
        auto it = m_byName.find(p_name);

        return it != m_byName.end() ? it->second : InvalidItemId;
    }

    ItemId ItemRegistry::Append(ItemDef &&p_def)
    {
        m_defs.push_back(std::move(p_def));

        const ItemId id{ static_cast<std::uint32_t>(m_defs.size()) };

        // The key views the name stored inside the deque element. Deque elements never
        // move on push_back, so the view stays valid. Never modify a registered name.
        m_byName.emplace(std::string_view(m_defs.back().name), id);

        return id;
    }
}