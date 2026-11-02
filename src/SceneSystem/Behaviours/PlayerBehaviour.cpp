#include "PlayerBehaviour.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <Node.hpp>
#include <SceneSystem/Items/Inventory.hpp>
#include <SceneSystem/Items/ItemRegistry.hpp>

namespace Droplet::Scene
{

    namespace
    {
        using Droplet::Game::ItemDef;
        using Droplet::Game::ItemId;
        using Droplet::Game::ItemRegistry;
        using Droplet::Game::ItemStack;
        using Droplet::Game::SavedStack;

        // [{ "item": "droplet:coolant_cell", "count": 16 }, ...]
        nlohmann::json InventoryToJson(const ItemRegistry &p_reg, const std::vector<ItemStack> &p_inv)
        {
            auto arr = nlohmann::json::array();
            for (const SavedStack &s : Droplet::Game::ToSaved(p_reg, p_inv))
                arr.push_back({ {"item", s.name}, {"count", s.count} });
            return arr;
        }

        std::vector<ItemStack> InventoryFromJson(const ItemRegistry &p_reg,
            const nlohmann::json &p_arr,
            std::size_t &p_skipped)
        {
            std::vector<SavedStack> saved;
            for (const auto &e : p_arr)
                saved.push_back({ e.value("item", std::string{}), e.value("count", 0u) });
            return Droplet::Game::FromSaved(p_reg, saved, &p_skipped);
        }

        /// Draws one inventory as a table: display name, stable name, count.
        /// Ids the registry doesn't know are shown greyed out instead of crashing.
        void RenderInventoryTable(const char *p_tableId,
            const ItemRegistry *p_reg,
            const std::vector<ItemStack> &p_inv)
        {
            if (!ImGui::BeginTable(
                p_tableId,
                3,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable))
            {
                return;
            }

            ImGui::TableSetupColumn("Item");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Count");
            ImGui::TableHeadersRow();

            for (const ItemStack &stack : p_inv)
            {
                const ItemDef *def = p_reg ? p_reg->Get(stack.item) : nullptr;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (def)
                    ImGui::TextUnformatted(def->displayName.c_str());
                else
                    ImGui::TextDisabled("<unknown #%u>", stack.item.value);

                ImGui::TableSetColumnIndex(1);
                if (def)
                    ImGui::TextUnformatted(def->name.c_str());
                else
                    ImGui::TextDisabled("-");

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%u", stack.count);
            }

            ImGui::EndTable();
        }

        /// Debug row: pick any registered item, choose an amount, press Add.
        /// Goes through PlayerBehaviour::AddItem so stack limits are respected.
        /// Selection state is static (shared between player behaviours), which is fine for a debug UI.
        void RenderAddItemRow(PlayerBehaviour &p_player, InventoryType p_type, const char *p_id)
        {
            const ItemRegistry *reg = p_player.GetItemRegistry();
            if (!reg)
            {
                ImGui::TextDisabled("No item registry set");
                return;
            }

            const std::size_t slot = static_cast<std::size_t>(p_type);
            static ItemId s_selected[2];
            static int s_count[2] = { 1, 1 };

            ImGui::PushID(p_id);

            const ItemDef *selectedDef = reg->Get(s_selected[slot]);

            ImGui::SetNextItemWidth(160.0f);
            if (ImGui::BeginCombo("##item", selectedDef ? selectedDef->displayName.c_str() : "Select item..."))
            {
                reg->ForEach([&](ItemId p_itemId, const ItemDef &p_def)
                    {
                        ImGui::PushID(static_cast<int>(p_itemId.value));
                        if (ImGui::Selectable(p_def.displayName.c_str(), p_itemId == s_selected[slot]))
                            s_selected[slot] = p_itemId;
                        ImGui::PopID();
                    });
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputInt("##count", &s_count[slot]);
            s_count[slot] = std::max(1, s_count[slot]);

            ImGui::SameLine();
            if (ImGui::Button("Add") && s_selected[slot].IsValid())
            {
                p_player.AddItem(p_type, s_selected[slot], static_cast<std::uint32_t>(s_count[slot]));
            }

            ImGui::PopID();
        }
    }

    std::uint32_t PlayerBehaviour::AddItem(InventoryType p_inv, Droplet::Game::ItemId p_id, std::uint32_t p_count)
    {
        const ItemRegistry *reg = GetItemRegistry();

        assert(reg != nullptr);
        assert(p_id.IsValid());

        if (!reg || !p_id.IsValid())
            return p_count;

        return Droplet::Game::AddItem(*reg, Inv(p_inv), p_id, p_count);
    }

    bool PlayerBehaviour::RemoveItem(InventoryType p_inv, Droplet::Game::ItemId p_id, std::uint32_t p_count)
    {
        return Droplet::Game::RemoveItem(Inv(p_inv), p_id, p_count);
    }

    std::uint32_t PlayerBehaviour::CountItem(InventoryType p_inv, Droplet::Game::ItemId p_id) const
    {
        return Droplet::Game::CountItem(Inv(p_inv), p_id);
    }

    nlohmann::json PlayerBehaviour::SerializeImpl()
    {
        const ItemRegistry *reg = GetItemRegistry();
        assert(reg && "SceneManager must have an item registry before serializing the player");

        nlohmann::json j;
        j["maxHealth"] = m_data.maxHealth;
        j["heatResistance"] = m_data.heatResistance;
        j["heatLevel"] = m_data.heatLevel;
        j["pressure"] = m_data.pressure;
        if (reg)
        {
            j["playerInventory"] = InventoryToJson(*reg, m_data.playerInventory);
            j["droneInventory"] = InventoryToJson(*reg, m_data.droneInventory);
        }
        return j;
    }

    void PlayerBehaviour::DeserializeImpl(const nlohmann::json p_behJson)
    {
        m_data.maxHealth = p_behJson.value("maxHealth", 100.f);
        m_data.heatResistance = p_behJson.value("heatResistance", 50.f);
        m_data.heatLevel = p_behJson.value("heatLevel", 10.f);
        m_data.pressure = p_behJson.value("pressure", 0.f);

        const ItemRegistry *reg = GetItemRegistry();
        assert(reg && "SceneManager must have an item registry before deserializing the player");
        if (!reg)
            return;

        std::size_t skipped = 0;
        m_data.playerInventory = InventoryFromJson(
            *reg, p_behJson.value("playerInventory", nlohmann::json::array()), skipped);
        m_data.droneInventory = InventoryFromJson(
            *reg, p_behJson.value("droneInventory", nlohmann::json::array()), skipped);

        if (skipped > 0)
        {
            // Log with your logger: "Player save referenced <skipped> unknown item(s), dropped."
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

        const ItemRegistry *reg = GetItemRegistry();

        // Drone inventory.
        if (ImGui::CollapsingHeader("Drone Inventory"))
        {
            RenderInventoryTable("DroneInventory", reg, m_data.droneInventory);
            RenderAddItemRow(*this, InventoryType::Drone, "DroneAdd");
        }

        // Player inventory.
        if (ImGui::CollapsingHeader("Player Inventory"))
        {
            RenderInventoryTable("PlayerInventory", reg, m_data.playerInventory);
            RenderAddItemRow(*this, InventoryType::Player, "PlayerAdd");
        }
    }
}