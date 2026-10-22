
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
    // Nothing to initialize for the temporary implementation.
    SetTarget(this->GetOwner());
    SetOffset({ 0.0f, 2.0f, -5.0f });
}

void CameraComponent::UpdateImpl([[maybe_unused]] float p_deltaTime)
{
    const auto target = m_target.lock();

    if (!target)
        return;

    // Get the target's world position, accounting for parent transforms.
    const glm::vec3 targetPosition =
        target->GetTransform().GetPosition(Transform::Space::World);

    // Position the editor camera at the target plus the configured offset.
    const glm::vec3 cameraPosition = targetPosition + m_offset;

    m_camera->SetPosition(cameraPosition);

    // Calculate the direction from the camera towards the target.
    const glm::vec3 direction = targetPosition - cameraPosition;

    // Avoid calculating orientation from a zero-length vector.
    if (glm::dot(direction, direction) < 1e-6f)
        return;

    const glm::vec3 forward = glm::normalize(direction);

    // Convert the direction vector into yaw and pitch angles in degrees.
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
    // Temporary implementation; no camera data is serialized yet.
    return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(
    [[maybe_unused]] nlohmann::json p_compJson)
{
    // Temporary implementation; nothing to deserialize yet.
}
