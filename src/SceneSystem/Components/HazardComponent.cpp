#include <SceneSystem/Components/HazardComponent.hpp>

using namespace Droplet::Scene;

void HazardComponent::RenderUIImpl()
{
    ImGui::DragFloat("Damage", &m_damage, 0.1f, 0.0f, FLT_MAX);
}

nlohmann::json HazardComponent::SerializeImpl()
{
    nlohmann::json json;
    
    json["damage"] = m_damage;
    
    return json;
}

void HazardComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (!p_compJson.contains("damage"))
    {
        throw std::runtime_error("HazardComponent missing required field 'damage'");
    }
    m_damage = p_compJson["damage"];
}
