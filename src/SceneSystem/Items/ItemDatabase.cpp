#include "ItemDatabase.hpp"

#include <fstream>
#include <limits>
#include <utility>

namespace Droplet::Game
{
    bool ItemDatabase::LoadFromFile(const std::string &p_filePath)
    {
        try
        {
            std::ifstream file(p_filePath);

            if (!file.is_open())
                return false;

            const nlohmann::json json = nlohmann::json::parse(file);

            if (!json.is_object() ||
                !json.contains("items") ||
                !json["items"].is_array())
            {
                return false;
            }

            std::unordered_map<std::string, ItemDefinition> loadedItems;

            for (const auto &itemJson : json["items"])
            {
                if (!itemJson.is_object() ||
                    !itemJson.contains("id") ||
                    !itemJson.contains("name"))
                {
                    return false;
                }

                if (!itemJson["id"].is_string() ||
                    !itemJson["name"].is_string())
                {
                    return false;
                }

                ItemDefinition item;
                item.id = itemJson["id"].get<std::string>();
                item.name = itemJson["name"].get<std::string>();
                item.description = itemJson.value("description", "");

                if (itemJson.contains("maxStackSize"))
                {
                    const auto &stackSizeJson = itemJson["maxStackSize"];

                    if (!stackSizeJson.is_number_unsigned() &&
                        !stackSizeJson.is_number_integer())
                    {
                        return false;
                    }

                    const auto stackSize = stackSizeJson.get<std::int64_t>();

                    if (stackSize <= 0 ||
                        static_cast<std::uint64_t>(stackSize) >
                        std::numeric_limits<std::uint32_t>::max())
                    {
                        return false;
                    }

                    item.maxStackSize = static_cast<std::uint32_t>(stackSize);
                }

                if (item.id.empty() || item.name.empty())
                    return false;

                // Insert the item and reject duplicate IDs.
                if (!loadedItems.emplace(item.id, std::move(item)).second)
                    return false;
            }

            // Replace the current database only after successful loading.
            m_items = std::move(loadedItems);

            return true;
        }
        catch (const std::exception &)
        {
            return false;
        }
    }

    const ItemDefinition *ItemDatabase::GetItem(
        std::string_view p_itemId) const
    {
        const auto it = m_items.find(std::string(p_itemId));

        if (it == m_items.end())
            return nullptr;

        return &it->second;
    }

    bool ItemDatabase::Contains(std::string_view p_itemId) const
    {
        return m_items.find(std::string(p_itemId)) != m_items.end();
    }

    std::size_t ItemDatabase::GetItemCount() const
    {
        return m_items.size();
    }

    void ItemDatabase::Clear()
    {
        m_items.clear();
    }
}