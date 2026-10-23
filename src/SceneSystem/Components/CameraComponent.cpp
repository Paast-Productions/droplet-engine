
#include "CameraComponent.hpp"

#include <SceneSystem/Node.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <ImGui/imgui.h>

#include <cmath>

using namespace Droplet::Scene;

Droplet::Scene::CameraComponent::CameraComponent()
{
}

void CameraComponent::StartImpl()
{
}

void CameraComponent::UpdateImpl([[maybe_unused]] float p_deltaTime)
{
    if (!m_camera)
        return;

    const auto owner = std::move(this->GetOwner());
    if (!owner)
        return;

    const auto &transform = owner->GetTransform();

    // Follow the node's world position.
    const glm::vec3 position =
        transform.GetPosition(Transform::Space::World);

    const glm::vec3 forward = transform.GetForward(Transform::Space::World);
    const glm::vec3 right = transform.GetRight(Transform::Space::World);
    const glm::vec3 up = transform.GetUp(Transform::Space::World);

    // Offset is local: x = right, y = up, z = forward
    const glm::vec3 worldOffset =
        right * m_offset.x + up * m_offset.y + forward * m_offset.z;

    m_camera->SetPosition(position + worldOffset);

    constexpr float radiansToDegrees = 180.0f / glm::pi<float>();

    const float yaw =
        std::atan2(forward.z, forward.x) * radiansToDegrees;

    const float pitch =
        std::asin(glm::clamp(forward.y, -1.0f, 1.0f))
        * radiansToDegrees;

    m_camera->SetOrientation(yaw, pitch);
}

void CameraComponent::RenderUIImpl()
{
    ImGui::DragFloat3("Offset", &m_offset.x, 0.1f);

    ImGui::SameLine();

    if (ImGui::Button("Reset"))
    {
        m_offset = { 0.0f, 0.0f, 0.0f };
    }
}

nlohmann::json CameraComponent::SerializeImpl()
{
    return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(
    [[maybe_unused]] nlohmann::json p_compJson)
{
}
