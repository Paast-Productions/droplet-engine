#include "CameraComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <Graphics/DebugDraw.hpp>
#include <ImGui/imgui.h>

using namespace Droplet::Scene;
using namespace Droplet::Graphics;
using namespace Droplet::Graphics::Debug;

void CameraComponent::StartImpl()
{
	// TODO: Construct camera and create render target
}

void CameraComponent::LateUpdateImpl()
{
	Transform &t = GetOwner()->GetTransform();

	m_camera.SetPosition(t.GetPosition(Transform::Space::World));
	m_camera.SetOrientation(t.GetRotation(Transform::Space::World));

	m_frustum = m_camera.GetFrustum();
}

void CameraComponent::QueueRenderImpl()
{
	// TODO
}

void CameraComponent::EditorUpdateImpl()
{
	if (!m_debugDraw)
	{
		return;
	}

	Transform &t = GetOwner()->GetTransform();

	m_camera.SetPosition(t.GetPosition(Transform::Space::World));
	m_camera.SetOrientation(t.GetRotation(Transform::Space::World));

	m_frustum = m_camera.GetFrustum();

	DebugDraw::DrawFrustum(
		m_frustum, 
		glm::vec4(0.5f, 0.5f, 0.5f, 0.7f),
		false,
		true
	);
}

void CameraComponent::RenderUIImpl()
{
	using namespace ImGui;

	// TODO
	// Render target view

	DrawLayerUtility::SetLayerMaskUI("Draw Layers", m_drawLayerMask);

	float fov = m_camera.GetFOV();
	if (SliderFloat("FOV", &fov, 1.0f, 179.0f))
	{
		m_camera.SetFOV(fov);
	}

	float aspect = m_camera.GetAspect();
	if (DragFloat("Aspect Ratio", &aspect, 0.01f))
	{
		m_camera.SetAspect(std::max(0.0001f, aspect));
	}

	glm::vec2 planes = m_camera.GetPlanes();
	bool planesChanged = false;
	planesChanged |= DragFloat("Near", &planes.x, 0.001f);
	planesChanged |= DragFloat("Far", &planes.y, 0.1f);
	if (planesChanged)
	{
		m_camera.SetPlanes(std::max(0.01f, planes.x), std::max(0.01f, planes.y));	
	}

	Checkbox("Debug Draw", &m_debugDraw);

}

nlohmann::json CameraComponent::SerializeImpl()
{
	// TODO
	return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(nlohmann::json p_compJson)
{
	// TODO
}
