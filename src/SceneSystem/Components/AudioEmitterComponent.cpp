#include "AudioEmitterComponent.hpp"
#include "ImGui/imgui.h"


using namespace Droplet::Scene;

void AudioEmitterComponent::RenderUIImpl()
{
	using namespace ImGui;

	if (Button("Play"))
	{
		// TODO: Get the audio emitter from the audio system and play it
	}
	SameLine();
	if (Button("Stop"))
	{
		// TODO: Get the audio emitter from the audio system and stop it
	}

	if (TreeNode("Audio Settings"))
	{
		SliderFloat("volume", &m_volume, 0.0f, 1.0f, "%.2f");

		Checkbox("Looping", &m_looping);

		TreePop();
	}
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
