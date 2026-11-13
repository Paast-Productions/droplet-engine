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

void HazardComponent::RenderUIImpl()
{
    ImGui::DragFloat("Damage", &m_damage, 0.1f, 0.0f, FLT_MAX);
    
    static bool drawDir = false;
    ImGui::Checkbox("Draw direction", &drawDir);
    static glm::vec3 dir = glm::vec3(0.0f, 0.0f, 1.0f);
    ImGui::DragFloat3("Direction", &dir.x, 0.1f, FLT_MIN, FLT_MAX);
    m_direction = dir;
    m_direction = GetDirection();
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
