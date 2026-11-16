#include <SceneSystem/Components/HazardComponent.hpp>
#include <SceneSystem/Node.hpp>
#include <Graphics/DebugDraw.hpp>

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
    std::string text = "Damage: " + std::to_string(m_damage) + "/s";
    ImGui::Text(text.c_str());
    
    std::string directionStr = "Direction (" + std::to_string(m_direction.x) + ", " + std::to_string(m_direction.y) + ", " + std::to_string(m_direction.z) + ")";
    ImGui::Text(directionStr.c_str());
    static bool drawDir = false;
    ImGui::Checkbox("Draw direction", &drawDir);
    if (drawDir)
    {
        glm::vec3 start = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
        Graphics::Debug::DebugDraw::DrawLine(start, start + m_direction, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true);
    }
}

nlohmann::json HazardComponent::SerializeImpl()
{
    nlohmann::json json;
    
    json["damage"] = m_damage;
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
        throw std::runtime_error("HazardComponent missing required field 'damage'");
    }
    m_damage = p_compJson["damage"];
    
    m_direction.x = p_compJson["direction"][0];
    m_direction.y = p_compJson["direction"][1];
    m_direction.z = p_compJson["direction"][2];
}
