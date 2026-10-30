
#include "PlayerBehaviour.hpp"
#include <algorithm>
#include <cstdint>
#include <Node.hpp>

namespace Droplet::Scene
{

    nlohmann::json PlayerBehaviour::SerializeImpl()
    {
        // Serialize an inventory into a JSON array.
        auto serializeInventory = [](const std::vector<Droplet::Game::ItemStack> &inventory)
            {
                nlohmann::json jsonInventory = nlohmann::json::array();

                for (const auto &item : inventory)
                {
                    jsonInventory.push_back({
                        {"itemId", item.itemId},
                        {"count", item.count}
                        });
                }

                return jsonInventory;
            };

        return {
            {"maxHealth", m_data.maxHealth},
            {"heatResistance", m_data.heatResistance},
            {"heatLevel", m_data.heatLevel},
            {"pressure", m_data.pressure},

            {"droneInventory", serializeInventory(m_data.droneInventory)},
            {"playerInventory", serializeInventory(m_data.playerInventory)}
        };
    }

    void PlayerBehaviour::DeserializeImpl(const nlohmann::json p_behJson)
    {
        // Restore player stats.
        m_data.maxHealth = p_behJson.value("maxHealth", 100.0f);
        m_data.heatResistance = p_behJson.value("heatResistance", 50.0f);
        m_data.heatLevel = p_behJson.value("heatLevel", 10.0f);
        m_data.pressure = p_behJson.value("pressure", 0.0f);

        // Deserialize an inventory from a JSON array.
        auto deserializeInventory = [](const nlohmann::json &jsonInventory,
            std::vector<Droplet::Game::ItemStack> &inventory)
            {
                inventory.clear();

                if (!jsonInventory.is_array())
                    return;

                for (const auto &item : jsonInventory)
                {
                    Droplet::Game::ItemStack stack;

                    stack.itemId = item.value("itemId", std::string{});
                    stack.count = item.value("count", std::uint32_t{ 0 });

                    inventory.push_back(std::move(stack));
                }
            };

        // Restore inventories.
        if (p_behJson.contains("droneInventory"))
        {
            deserializeInventory(
                p_behJson.at("droneInventory"),
                m_data.droneInventory);
        }
        else
        {
            m_data.droneInventory.clear();
        }

        if (p_behJson.contains("playerInventory"))
        {
            deserializeInventory(
                p_behJson.at("playerInventory"),
                m_data.playerInventory);
        }
        else
        {
            m_data.playerInventory.clear();
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

        ImGui::InputFloat(
            "Heat Level",
            &m_data.heatLevel,
            0.1f,
            1.0f,
            "%.2f");

        m_data.heatLevel = std::max(0.0f, m_data.heatLevel);

        ImGui::Spacing();
        ImGui::Separator();

        // Drone inventory.
        if (ImGui::CollapsingHeader("Drone Inventory"))
        {
            if (ImGui::BeginTable(
                "DroneInventory",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Item ID");
                ImGui::TableSetupColumn("Count");
                ImGui::TableHeadersRow();

                for (const auto &item : m_data.droneInventory)
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

        // Player inventory.
        if (ImGui::CollapsingHeader("Player Inventory"))
        {
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

                for (const auto &item : m_data.playerInventory)
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
}

