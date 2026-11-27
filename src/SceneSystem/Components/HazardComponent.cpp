#include <SceneSystem/Components/HazardComponent.hpp>
#include <SceneSystem/Node.hpp>
//#include <Graphics/DebugDraw.hpp>

using namespace Droplet::Scene;

glm::vec3 HazardComponent::GetDirection()
{
    if (glm::length(m_direction) == 0.0f)
    {
        m_direction = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    return glm::normalize(m_direction);
}

void HazardComponent::SetDirection(glm::vec3 p_direction)
{
    if (glm::length(p_direction) == 0.0f)
    {
        p_direction = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    m_direction = glm::normalize(p_direction);
}

void HazardComponent::RenderUIImpl()
{
#ifdef _EDITOR
    //std::string text = "Damage: " + std::to_string(m_damage) + "/s";
    //ImGui::Text(text.c_str());
    ImGui::DragFloat("damage", &m_damage, 1.0f, 0.0f, 100.0f);
    
    m_damage = std::max(0.5f, m_damage);
    
    std::string directionStr = "Direction (" + std::to_string(m_direction.x) + ", " + std::to_string(m_direction.y) + ", " + std::to_string(m_direction.z) + ")";
    ImGui::Text(directionStr.c_str());
    static bool drawDir = false;
    ImGui::Checkbox("Draw direction", &drawDir);
    if (drawDir)
    {
        glm::vec3 start = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
        Graphics::Debug::DebugDraw::DrawLine(start, start + m_direction, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true);
    }

    ImGui::RadioButton("Zone 1", &m_zone, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 2", &m_zone, 2);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 3", &m_zone, 3);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 4", &m_zone, 4);
    ImGui::SameLine();
    ImGui::RadioButton("Zone 5", &m_zone, 5);
#endif
}

nlohmann::json HazardComponent::SerializeImpl()
{
    nlohmann::json json;
    
    json["damage"] = m_damage;
    json["zone"] = m_zone;
    json["direction"][0] = m_direction.x;
    json["direction"][1] = m_direction.y;
    json["direction"][2] = m_direction.z;
    
    return json;
}

void HazardComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (!p_compJson.contains("damage"))
    {
        throw std::runtime_error("HazardComponent missing required field 'damage'");
    }
    if (!p_compJson.contains("direction"))
    {
        throw std::runtime_error("HazardComponent missing required field 'direction'");
    }
    m_damage = p_compJson["damage"];
    
    m_direction.x = p_compJson["direction"][0];
    m_direction.y = p_compJson["direction"][1];
    m_direction.z = p_compJson["direction"][2];

    if (p_compJson.contains("zone"))
    {
        m_zone = p_compJson["zone"];

        //throw std::runtime_error("HazardComponent missing required field 'zone'");
    }
}
