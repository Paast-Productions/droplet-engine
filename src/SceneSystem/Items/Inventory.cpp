#include <SceneSystem/Items/Inventory.hpp>

#include <algorithm>

#include <SceneSystem/Items/ItemRegistry.hpp>

namespace Droplet::Game
{
    std::uint32_t CountItem(
        const std::vector<ItemStack> &p_inventory,
        ItemId p_id)
    {
        std::uint32_t total = 0;

        for (const ItemStack &stack : p_inventory)
        {
            if (stack.item == p_id)
                total += stack.count;
        }

        return total;
    }

    std::uint32_t AddItem(
        const ItemRegistry &p_registry,
        std::vector<ItemStack> &p_inventory,
        ItemId p_id,
        std::uint32_t p_count,
        std::size_t p_maxSlots)
    {
        const ItemDef *def = p_registry.Get(p_id);
        if (!def || p_count == 0)
            return p_count;

        const std::uint32_t maxStack = std::max<std::uint32_t>(1, def->maxStack);

        for (ItemStack &stack : p_inventory)
        {
            if (stack.item != p_id || stack.count >= maxStack)
                continue;

            const std::uint32_t moved = std::min(maxStack - stack.count, p_count);
            stack.count += moved;
            p_count -= moved;

            if (p_count == 0)
                return 0;
        }

        while (p_count > 0 && p_inventory.size() < p_maxSlots)
        {
            const std::uint32_t moved = std::min(maxStack, p_count);
            p_inventory.push_back(ItemStack{ p_id, moved });
            p_count -= moved;
        }

        return p_count;
    }

    bool RemoveItem(
        std::vector<ItemStack> &p_inventory,
        ItemId p_id,
        std::uint32_t p_count)
    {
        if (CountItem(p_inventory, p_id) < p_count)
            return false;

        for (auto it = p_inventory.rbegin(); it != p_inventory.rend() && p_count > 0; ++it)
        {
            if (it->item != p_id)
                continue;

            const std::uint32_t taken = std::min(it->count, p_count);
            it->count -= taken;
            p_count -= taken;
        }

        p_inventory.erase(
            std::remove_if(
                p_inventory.begin(),
                p_inventory.end(),
                [](const ItemStack &stack)
                {
                    return stack.count == 0;
                }),
            p_inventory.end());

        return true;
    }

    std::vector<SavedStack> ToSaved(
        const ItemRegistry &p_registry,
        const std::vector<ItemStack> &p_inventory)
    {
        std::vector<SavedStack> saved;
        saved.reserve(p_inventory.size());

        for (const ItemStack &stack : p_inventory)
        {
            if (p_registry.Get(stack.item))
                saved.push_back({ p_registry.NameOf(stack.item), stack.count });
        }

        return saved;
    }

    std::vector<ItemStack> FromSaved(
        const ItemRegistry &p_registry,
        const std::vector<SavedStack> &p_saved,
        std::size_t *p_skipped)
    {
        std::vector<ItemStack> inventory;
        inventory.reserve(p_saved.size());

        for (const SavedStack &stack : p_saved)
        {
            const ItemId id = p_registry.Find(stack.name);

            if (id.IsValid())
                inventory.push_back({ id, stack.count });
            else if (p_skipped)
                ++*p_skipped;
        }

        return inventory;
    }
}