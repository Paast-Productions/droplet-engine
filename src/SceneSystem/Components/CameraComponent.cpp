
#include "CameraComponent.hpp"

#include <SceneSystem/Node.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <ImGui/imgui.h>
#include <Time.hpp>
#include <cmath>

namespace
{
    float WrapDegrees(float a)
    {
        while (a > 180.0f)  a -= 360.0f;
        while (a < -180.0f) a += 360.0f;
        return a;
    }

    
    // Rate: higher = snappier. At or above kInstantRate there is no smoothing.
    constexpr float kInstantRate = 500.0f;

    float Smooth(float rate, float dt)
    {
        if (rate >= kInstantRate)
            return 1.0f;                        // no delay
        return 1.0f - std::exp(-rate * dt);
    }
}

using namespace Droplet::Scene;

Droplet::Scene::CameraComponent::CameraComponent()
{
}

void CameraComponent::StartImpl()
{
}

void CameraComponent::UpdateImpl(float p_deltaTime)
{
    if (!m_camera)
        return;

    const auto owner = this->GetOwner();
    if (!owner)
        return;

    const auto &transform = owner->GetTransform();

    const glm::vec3 dronePos = transform.GetPosition(Transform::Space::World);
    const glm::vec3 forward = transform.GetForward(Transform::Space::World);

    constexpr float radToDeg = 180.0f / glm::pi<float>();
    constexpr float degToRad = glm::pi<float>() / 180.0f;

    const float targetYaw = std::atan2(forward.z, forward.x) * radToDeg;
    const float targetPitch = std::asin(glm::clamp(forward.y, -1.0f, 1.0f)) * radToDeg;

    // Snap on the first frame so the camera doesn't fly in from the origin
    if (!m_initialized)
    {
        m_smoothedYaw = targetYaw;
        m_smoothedPitch = targetPitch;
        m_smoothedPosition = dronePos;
        m_initialized = true;
    }

    // Smooth orientation (shortest path for yaw)
    const float rotT = Smooth(m_rotationLag, p_deltaTime);
    m_smoothedYaw += WrapDegrees(targetYaw - m_smoothedYaw) * rotT;
    m_smoothedPitch += (targetPitch - m_smoothedPitch) * rotT;

    // Build the camera's frame from the SMOOTHED orientation
    const float yawR = m_smoothedYaw * degToRad;
    const float pitchR = m_smoothedPitch * degToRad;

    const glm::vec3 camForward{
        std::cos(pitchR) * std::cos(yawR),
        std::sin(pitchR),
        std::cos(pitchR) * std::sin(yawR) };

    const glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0, 1, 0)));
    const glm::vec3 camUp = glm::cross(camRight, camForward);

    // Offset lives in the smoothed frame, so the drone swings within the view
    const glm::vec3 worldOffset =
        camRight * m_offset.x + camUp * m_offset.y + camForward * m_offset.z;

    // Smooth position too (small lag makes acceleration visible)
    const float posT = Smooth(m_positionLag, p_deltaTime);
    m_smoothedPosition += (dronePos - m_smoothedPosition) * posT;

    m_camera->SetPosition(m_smoothedPosition + worldOffset);
    m_camera->SetOrientation(m_smoothedYaw, m_smoothedPitch);
}

void CameraComponent::RenderUIImpl()
{

    ImGui::DragFloat3("Offset", &m_offset.x, 0.1f);
    ImGui::DragFloat("Rotation Lag", &m_rotationLag, 1.0f, 0.5f, 500.0f);
    ImGui::DragFloat("Position Lag", &m_positionLag, 1.0f, 0.5f, 500.0f);
}

nlohmann::json CameraComponent::SerializeImpl()
{
    return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(
    [[maybe_unused]] nlohmann::json p_compJson)
{
}
