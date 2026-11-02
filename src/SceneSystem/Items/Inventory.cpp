#include <SceneSystem/Items/Inventory.hpp>

#include <algorithm>

#include <SceneSystem/Items/ItemRegistry.hpp>

namespace Droplet::Game
{
    std::uint32_t CountItem(const std::vector<ItemStack> &inv, ItemId id)
    {
        std::uint32_t total = 0;
        for (const ItemStack &s : inv)
            if (s.item == id)
                total += s.count;
        return total;
    }

    std::uint32_t AddItem(const ItemRegistry &reg,
        std::vector<ItemStack> &inv,
        ItemId id,
        std::uint32_t count,
        std::size_t maxSlots)
    {
        const ItemDef *def = reg.Get(id);
        if (!def || count == 0)
            return count;

        const std::uint32_t maxStack = std::max<std::uint32_t>(1, def->maxStack);

        for (ItemStack &s : inv)
        {
            if (s.item != id || s.count >= maxStack)
                continue;
            const std::uint32_t moved = std::min(maxStack - s.count, count);
            s.count += moved;
            count -= moved;
            if (count == 0)
                return 0;
        }

        while (count > 0 && inv.size() < maxSlots)
        {
            const std::uint32_t moved = std::min(maxStack, count);
            inv.push_back(ItemStack{ id, moved });
            count -= moved;
        }
        return count;
    }

    bool RemoveItem(std::vector<ItemStack> &inv, ItemId id, std::uint32_t count)
    {
        if (CountItem(inv, id) < count)
            return false;

        for (auto it = inv.rbegin(); it != inv.rend() && count > 0; ++it)
        {
            if (it->item != id)
                continue;
            const std::uint32_t taken = std::min(it->count, count);
            it->count -= taken;
            count -= taken;
        }

        inv.erase(std::remove_if(inv.begin(), inv.end(),
            [](const ItemStack &s) { return s.count == 0; }),
            inv.end());
        return true;
    }

    std::vector<SavedStack> ToSaved(const ItemRegistry &reg, const std::vector<ItemStack> &inv)
    {
        std::vector<SavedStack> out;
        out.reserve(inv.size());
        for (const ItemStack &s : inv)
            if (reg.Get(s.item))
                out.push_back({ reg.NameOf(s.item), s.count });
        return out;
    }

    std::vector<ItemStack> FromSaved(const ItemRegistry &reg,
        const std::vector<SavedStack> &saved,
        std::size_t *skipped)
    {
        std::vector<ItemStack> out;
        out.reserve(saved.size());
        for (const SavedStack &s : saved)
        {
            const ItemId id = reg.Find(s.name);
            if (id.IsValid())
                out.push_back({ id, s.count });
            else if (skipped)
                ++*skipped;
        }
        return out;
    }
}