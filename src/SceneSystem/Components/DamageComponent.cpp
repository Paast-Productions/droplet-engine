#include <SceneSystem/Components/DamageComponent.hpp>

using namespace Droplet::Scene;

void DamageComponent::RenderUIImpl()
{
    ImGui::DragFloat("Damage", &m_damage, 0.1f, 0.0f, FLT_MAX);
}

nlohmann::json DamageComponent::SerializeImpl()
{
    nlohmann::json json;
    
    json["damage"] = m_damage;
    
    return json;
}

void DamageComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (!p_compJson.contains("damage"))
    {
        throw std::runtime_error("DamageComponent missing required field 'damage'");
    }
    m_damage = p_compJson["damage"];
}
