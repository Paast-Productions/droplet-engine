#include "AudioEmitterComponent.hpp"
#include "ImGui/imgui.h"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Audio/AudioSystem.hpp>
#include <Audio/AudioBus.hpp>
#include <SceneSystem/Behaviour.hpp>
using namespace Droplet::Scene;

Droplet::Scene::AudioEmitterComponent::AudioEmitterComponent()
{
}

void Droplet::Scene::AudioEmitterComponent::StartImpl()
{
	m_mixer = &Audio::AudioSystem::GetInstance().GetMixer();
}

void AudioEmitterComponent::RenderUIImpl()
{
	ImVec2 buttonSize = ImVec2(30, 30);
	if (ImGui::Button(ICON_LC_PLAY "##PlaySoundButton",buttonSize))
	{
		PlaySound();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_LC_PAUSE "##PausesoundButton", buttonSize))
	{
		// TODO: Get the audio emitter from the audio system and stop it
	}

	if (ImGui::TreeNode("Audio Settings" ICON_LC_SETTINGS))
	{
		ImGui::SliderFloat("Volume " ICON_LC_VOLUME_2, &m_volume, 0.0f, 1.0f, "%.2f");

		ImGui::SliderFloat("Pitch", &m_pitch, 0.0f, 10.0f, " % .2f");
		
		ImGui::Checkbox("Looping", &m_looping);


		ImGui::DragFloatRange2("Sound Distance", &m_minDistance, &m_maxDistance, 0.1f, 0.0f, 100.0f, "Min: %.1f", "Max: %.1f");

		ImGui::TreePop();
	}

	const std::string preview = m_audioBusName.empty() ? "Drop Audio Bus Here" : m_audioBusName;
	ImGui::Button(preview.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("AUDIO_BUS");
		

		if (payload != nullptr && payload->IsDelivery() && payload->DataSize == sizeof(std::uint64_t))
		{
			std::uint64_t draggedBusID = 0;
			std::memcpy(&draggedBusID, payload->Data, sizeof(draggedBusID));

			Audio::AudioBus *draggedBus = m_mixer->FindAudioBus(draggedBusID);

			if (draggedBus != nullptr)
			{
				m_audioBusName = draggedBus->GetName();
			}
		}
		ImGui::EndDragDropTarget();
	}


	if (!m_mixer->HasAudioBusWithName(m_audioBusName))
	{
		m_audioBusName = "Drop Audio Bus Here";
	}

	dalia::AttenuationCurve curves[] = { dalia::AttenuationCurve::InverseSquare,dalia::AttenuationCurve::Linear, dalia::AttenuationCurve::Quadratic };
	const char *items[] = { "Inverse Square","Linear", "Quadratic"};
	int current = 0;

	for (int i = 0; i < IM_COUNTOF(items); ++i)
	{
		if (curves[i] == m_attCurve)
		{
			current = i;
			break;
		}
	}


	ImGui::SeparatorText("Attenuation Curve");
	if (ImGui::Combo("##AttenuationCurve", &current, items, IM_COUNTOF(items)))
	{
		m_attCurve = curves[current];
	}
}

nlohmann::json Droplet::Scene::AudioEmitterComponent::SerializeImpl()
{
	
	nlohmann::json json;

	json["volume"] = m_volume;
	json["looping"] = m_looping;
	json["attenuationCurve"] = m_attCurve;

	json["minDistance"] = m_minDistance;
	json["maxDistance"] = m_maxDistance;

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

	if (p_compJson.contains("attenuationCurve"))
	{
		m_attCurve = p_compJson["attenuationCurve"];
	}

	if (p_compJson.contains("minDistance"))
	{
		m_minDistance = p_compJson["minDistance"];
	}

	if (p_compJson.contains("maxDistance"))
	{
		m_maxDistance = p_compJson["maxDistance"];
	}
}
	

void Droplet::Scene::AudioEmitterComponent::PlaySound()
{
	
	std::cout << "Playing sound" << std::endl;
}
