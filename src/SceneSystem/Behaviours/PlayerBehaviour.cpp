
#include "PlayerBehaviour.hpp"
#include <algorithm>
#include <cstdint>
#include <Node.hpp>

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
    void PlayerBehaviour::RenderUIImpl()
    {
        ImGui::TextUnformatted("Player Data");
        ImGui::Separator();

        // Player node status.
        if (auto player = GetPlayer())
        {
            ImGui::Text("Player Node: %s", player->GetName().c_str());
        }
        else
        {
            ImGui::TextDisabled("Player Node: Not assigned");
        }

        ImGui::Spacing();

        // Editable player stats.
        ImGui::TextUnformatted("Stats");

        int maxHealth = static_cast<int>(m_data.maxHealth);

        if (ImGui::InputInt("Max Health", &maxHealth))
        {
            maxHealth = std::max(1, maxHealth);
            m_data.maxHealth = static_cast<std::uint32_t>(maxHealth);
        }

        ImGui::InputFloat(
            "Heat Resistance",
            &m_data.heatResistance,
            0.1f,
            1.0f,
            "%.2f");

        m_data.heatResistance = std::max(0.0f, m_data.heatResistance);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Inventory (%zu items)", m_data.inventory.size());

        if (ImGui::BeginTable(
            "PlayerInventory",
            2,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Item ID");
            ImGui::TableSetupColumn("Count");
            ImGui::TableHeadersRow();

            for (const auto &item : m_data.inventory)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(item.itemId.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", item.count);
            }

            ImGui::EndTable();
        }
    }
}

