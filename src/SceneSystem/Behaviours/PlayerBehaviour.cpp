
#include "PlayerBehaviour.hpp"

namespace Droplet::Scene
{

    nlohmann::json PlayerBehaviour::SerializeImpl()
    {
        nlohmann::json inventory = nlohmann::json::array();

        for (const auto &item : m_data.inventory)
        {
            inventory.push_back({
                {"itemId", item.itemId},
                {"count", item.count}
                });
        }

        return {
            {"inventory", inventory},
            {"maxHealth", m_data.maxHealth}
        };
    }

    void PlayerBehaviour::DeserializeImpl(
        const nlohmann::json p_behJson)
    {
        m_data.maxHealth = p_behJson.value("maxHealth", std::uint32_t{ 3 });

        m_data.inventory.clear();

        if (p_behJson.contains("inventory"))
        {
            for (const auto &item : p_behJson.at("inventory"))
            {
                Droplet::Game::ItemStack stack;
                stack.itemId = item.at("itemId").get<std::string>();
                stack.count = item.at("count").get<std::uint32_t>();

                m_data.inventory.push_back(std::move(stack));
            }
        }
    }
}
