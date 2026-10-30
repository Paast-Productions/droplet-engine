#include "CameraComponent.hpp"

#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <Graphics/DebugDraw.hpp>
#include <ImGui/imgui.h>
#include <IntermediaryStorage.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Graphics;
using namespace Droplet::Graphics::Debug;

void CameraComponent::StartImpl()
{
	// TODO: Construct camera and create render target

	using namespace Graphics::VK;
	IntermediaryStorage &instance = IntermediaryStorage::GetInstance();
	m_targetIndex = instance.CreateRenderTarget(m_resolution.x, m_resolution.y);

	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
	{
		m_textureIndex = instance.CreateImGuiTexture(
			instance.GetRenderTargetAt(m_targetIndex)
		);
	}
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
	if (m_dirty)
	{
		// TODO

		Graphics::VK::IntermediaryStorage::GetInstance().UpdateImGuiRenderTarget(
			m_targetIndex, m_textureIndex,
			m_resolution.x, m_resolution.y
		);

		m_dirty = false;
	}

	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipAutoRender) || !m_autoRender)
	{
		if (!m_manualRender)
		{
			return;
		}
	}

	m_manualRender = false;

	std::weak_ptr<Scene> scene = GetOwner()->GetScene();
	std::shared_ptr<Component> comp = shared_from_this();
	std::shared_ptr<CameraComponent> cameraComp = std::dynamic_pointer_cast<CameraComponent>(comp);

	scene.lock()->QueueRenderer(cameraComp);
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

	DrawLayerUtility::SetLayerMaskUI("Draw Layers", m_drawLayerMask);

	InputInt("Priority", &m_priority);

	float fov = m_camera.GetFOV();
	if (SliderFloat("FOV", &fov, 1.0f, 179.0f))
	{
		m_camera.SetFOV(fov);
	}

	glm::ivec2 res = m_resolution;
	if (DragInt2("Resolution", &res.x, 0.25f))
	{
		res = glm::max(res, glm::ivec2(1, 1));
		SetResolution(res);
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

	if (Button("Trigger Render"))
	{
		TriggerRender();
	}

	Separator();

	ImTextureRef camViewTexture = static_cast<VkDescriptorSet>(
		Graphics::VK::IntermediaryStorage::GetInstance().GetImGuiTextureAt(m_textureIndex)
	);

	// Render the camera view texture to the scene view window
	ImGui::Image(camViewTexture, ImVec2(static_cast<float>(m_resolution.x), static_cast<float>(m_resolution.y)), ImVec2(0, 0), ImVec2(1, 1));
}

void CameraComponent::SetResolution(glm::ivec2 p_resolution)
{
	if (p_resolution.x <= 0 || p_resolution.y <= 0)
	{
		throw std::invalid_argument("Resolution must be greater than zero.");
	}

	m_resolution = p_resolution;
	m_camera.SetAspect(static_cast<float>(p_resolution.x) / static_cast<float>(p_resolution.y));
	m_dirty = true;
}

void CameraComponent::CullRenderables(const std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Component>>> &p_renderables)
{
	auto visibleComponents = GetOwner()->GetScene()->GetNodesIntersectingFrustum(m_frustum);

	// Filter the renderables to only include those that are visible in the camera's frustum
	for (const auto &[node, component] : p_renderables)
	{
		if (!node || !component)
		{
			continue;
		}

		if (!DrawLayerUtility::IsLayerEnabled(node->GetDrawLayer(), m_drawLayerMask))
		{
			continue;
		}

		// Check if the node is in the list of visible components
		auto it = std::find_if(visibleComponents.begin(), visibleComponents.end(),
			[&node](const std::weak_ptr<Droplet::Scene::Node> &weakNode) {
				auto sharedNode = weakNode.lock();
				return sharedNode && sharedNode == node;
			});

		if (it != visibleComponents.end())
		{
			m_renderData.push_back(component->GetRenderData());
		}
	}
}

CameraRenderData CameraComponent::GetRenderData() const
{
	CameraRenderData renderData{ };
	renderData.camera = m_camera;
	renderData.renderTarget = &IntermediaryStorage::GetInstance().GetRenderTargetAt(m_targetIndex);
	renderData.renderData = m_renderData;

	return renderData;
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
