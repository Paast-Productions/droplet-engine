#include "BusInspectorWindow.hpp"


void Droplet::Editor::BusInspectorWindow::SetCurrentBus(Audio::AudioBus *p_bus)
{
	m_currentBus = p_bus;
}

void Droplet::Editor::BusInspectorWindow::InitImpl()
{
	SetName("Bus Inspector");
}

void Droplet::Editor::BusInspectorWindow::CloseImpl()
{
}

void Droplet::Editor::BusInspectorWindow::RenderImpl()
{
	const Audio::AudioMixer &mixer = m_editorContext->GetAudioSystem().GetMixer();

	AudioMixerEditorState &state = m_editorContext->GetAudioMixerState();

	if (state.selectedAudioBusID.has_value())
	{
		m_currentBus = mixer.FindAudioBus(state.selectedAudioBusID.value());
	}
	if (m_currentBus == nullptr)
	{
		m_currentBus = mixer.GetMasterBus();
	}
	//Push IDS
	ImGui::PushID(m_currentBus);

	//name
	std::string busName = m_currentBus->GetName();
	if (ImGui::InputText("Bus Name", &busName))
	{
		m_currentBus->SetName(busName);
	}

	//Active
	bool active = m_currentBus->IsActive();
	if (ImGui::Checkbox("Active", &active))
	{
		m_currentBus->SetActive(active);
	}

	//Volume
	float volume = m_currentBus->GetVolume();
	if (ImGui::SliderFloat("Bus Volume", &volume, 0.0f, 2.0f, "%.3f"))
	{
		m_currentBus->SetVolume(volume);
	}


	ImGui::PopID();
}
