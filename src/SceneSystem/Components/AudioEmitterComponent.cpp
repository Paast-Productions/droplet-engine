#include "AudioEmitterComponent.hpp"
#include "ImGui/imgui.h"
#include <dalia/dalia.h>
using namespace Droplet::Scene;
AudioEmitterComponent::AudioEmitterComponent()
{
}

void AudioEmitterComponent::StartImpl()
{

}

void Droplet::Scene::AudioEmitterComponent::UpdateImpl(float deltaTime)
{
}

void Droplet::Scene::AudioEmitterComponent::RenderUIImpl()
{

	ImGui::Checkbox("2D", &m_is2D);
	ImGui::Checkbox("3D", &m_is3D);
	if (ImGui::TreeNode("Audio Settings"))
	{
		ImGui::SliderFloat("volume", &m_volume, 0.0f, 1.0f, "%.2f");
		//DOTO GET THE MESHES POSITION AND USE THAT FOR THE AUDIO EMITTER POSITION
		//m_position = is a temp for just testing the UI,
		// need to be replaced with the mesh position later

		ImGui::DragFloat3("position for 3D audio", &m_position.x, 0.1f);

		ImGui::TreePop();

		if (ImGui::Button("play"))
		{
			//TODO: Get the audio emitter from the audio system and play it
		}
		if (ImGui::Button("stop"))
		{
			//TODO: Get the audio emitter from the audio system and stop it
		}
		ImGui::Checkbox("Is Looping", &m_looping);
	}
}

nlohmann::json Droplet::Scene::AudioEmitterComponent::SerializeImpl()
{
	
	nlohmann::json json;

	json["volume"] = m_volume;
	json["looping"] = m_looping;
	json["2D"] = m_is2D;
	json["3D"] = m_is3D;

	return json;
}

void Droplet::Scene::AudioEmitterComponent::DeserializeImpl(nlohmann::json p_compJson)
{
	if (p_compJson.contains("volume"))
	{
		m_volume = p_compJson["volume"];
	}
	if (p_compJson.contains("looping"))
	{
		m_looping = p_compJson["looping"];
	}
	if (p_compJson.contains("2D"))
	{
		m_is2D = p_compJson["2D"];
	}
	if (p_compJson.contains("3D"))
	{
		m_is3D = p_compJson["3D"];
	}
}
