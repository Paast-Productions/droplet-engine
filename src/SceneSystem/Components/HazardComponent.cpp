#include <SceneSystem/Components/HazardComponent.hpp>

using namespace Droplet::Scene;

void HazardComponent::RenderUIImpl()
{
    ImGui::DragFloat("Damage", &m_damage, 0.1f, 0.0f, FLT_MAX);

    ImGui::RadioButton("Zone 1", &m_zone, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 2", &m_zone, 2);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 3", &m_zone, 3);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 4", &m_zone, 4);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 5", &m_zone, 5);
}

nlohmann::json HazardComponent::SerializeImpl()
{
    nlohmann::json json;
    
    json["damage"] = m_damage;
    json["zone"] = m_zone;
    
    return json;
}

void HazardComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (!p_compJson.contains("damage"))
    {
        throw std::runtime_error("HazardComponent missing required field 'damage'");
    }
    m_damage = p_compJson["damage"];

    if (p_compJson.contains("zone"))
    {
        m_zone = p_compJson["zone"];

        //throw std::runtime_error("HazardComponent missing required field 'zone'");
    }
}
