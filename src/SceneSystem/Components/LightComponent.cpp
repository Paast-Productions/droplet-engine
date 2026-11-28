#include "LightComponent.hpp"

#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <Common/RenderData.hpp>
#include <Graphics/Debug/DebugDraw.hpp>
#include <ImGui/imgui.h>

namespace Droplet::Scene
{

	void LightComponent::RenderUIImpl()
	{
		/*ImGui::DragFloat("Red", &m_color.x, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Green", &m_color.y, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Blue", &m_color.z, 0.05f, 0.0f, 1.0f);*/

		ImGui::ColorEdit3("Color", &m_color.x);

		ImGui::DragFloat("Intensity", &m_intensity, 0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Range", &m_range, 0.1f, 0.0f, FLT_MAX);
	}

	glm::vec3 LightComponent::GetColor()
	{
		return m_color;
	}
	float LightComponent::GetIntensity()
	{
		return m_intensity;
	}
	float LightComponent::GetRange()
	{
		return m_range;
	}

	void LightComponent::QueueRenderImpl()
	{
		glm::vec3 pos = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
		
		Common::LightData lightData{};
		lightData.colorAndIntensity = { m_color.r, m_color.g, m_color.b, m_intensity };
		lightData.positionAndRange = { pos.x, pos.y, pos.z, m_range };

		GetOwner()->GetScene()->QueueLight(lightData);
	}

	nlohmann::json LightComponent::SerializeImpl()
	{
		nlohmann::json json;

		json["color"] = { m_color.r, m_color.g, m_color.b };

		json["intensity"] = m_intensity;
		json["range"] = m_range;

		return json;
	}

	void LightComponent::DeserializeImpl(const nlohmann::json p_compJson)
	{
		if (p_compJson.contains("color"))
		{
			auto& color = p_compJson["color"];
			if (color.is_array() && color.size() == 3)
			{
				m_color.r = color[0].get<float>();
				m_color.g = color[1].get<float>();
				m_color.b = color[2].get<float>();
			}

		}

		m_intensity = p_compJson["intensity"];
		m_range = p_compJson["range"];

	}

}
