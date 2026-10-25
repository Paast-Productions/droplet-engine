
#include "CameraComponent.hpp"

#include <SceneSystem/Node.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>

#include <cmath>

using namespace Droplet::Scene;

Droplet::Scene::CameraComponent::CameraComponent()
{
}

void CameraComponent::StartImpl()
{
}

void CameraComponent::UpdateImpl()
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

    m_camera->SetPosition(position);

    // Use the node's actual world-space forward direction.
    const glm::vec3 forward =
        transform.GetForward(Transform::Space::World);

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
    
}

nlohmann::json CameraComponent::SerializeImpl()
{
    return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(
    [[maybe_unused]] nlohmann::json p_compJson)
{
}
