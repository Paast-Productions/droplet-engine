#include "AudioEmitterComponent.hpp"
#include "ImGui/imgui.h"
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

	ImGui::SliderFloat("volume", &m_volume, 0.0f, 1.0f);

	
	ImGui::Checkbox("Is Looping", &m_looping);


}

nlohmann::json Droplet::Scene::AudioEmitterComponent::SerializeImpl()
{
	
	nlohmann::json json;

	json["volume"] = m_volume;
	json["looping"] = m_looping;

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
}
