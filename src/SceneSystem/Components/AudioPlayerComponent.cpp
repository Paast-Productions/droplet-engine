#include "AudioPlayerComponent.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>

Droplet::Scene::AudioPlayerComponent::AudioPlayerComponent()
{
}

void Droplet::Scene::AudioPlayerComponent::StartImpl()
{
	m_mixer = &Audio::AudioSystem::GetInstance().GetMixer();
}

void Droplet::Scene::AudioPlayerComponent::RenderUIImpl()
{
	if (ImGui::Button("Play"))
	{
		//TODO: play the sound
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause"))
	{
		//TODO: Pause the sound
	}

	if (ImGui::TreeNode("Audio Settings"))
	{
		ImGui::SliderFloat("volume", &m_volume, 0.0f, 1.0f, "%.2f");

		ImGui::DragFloat("Pitch", &m_pitch, 0.1f, 0.0f, 2.0f, "%.2f");

		ImGui::Checkbox("Looping", &m_looping);

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

}

nlohmann::json Droplet::Scene::AudioPlayerComponent::SerializeImpl()
{
	return nlohmann::json();
}

void Droplet::Scene::AudioPlayerComponent::DeserializeImpl(nlohmann::json p_compJson)
{
}
