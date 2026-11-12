#include "BusInspectorWindow.hpp"
#include <Audio/EffectFactory.hpp>
#include <EffectList.hpp>
#include <Audio/Effect.hpp>
#include <Audio/AudioBus.hpp>


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
	const Audio::AudioMixer &mixer = Audio::AudioSystem::GetInstance().GetMixer();

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

	const float buttonWidth = ImGui::GetContentRegionAvail().x;
	const float buttonHeight = ImGui::GetTextLineHeight() * 1.5f;
	const float buttonMargin = buttonWidth * 0.2f;

	const auto& effects = m_currentBus->GetAllEffects();

	for (uint32_t effectIndex = 0; effectIndex < effects.size(); ++effectIndex)
	{
		const auto &effect = effects[effectIndex];
		if (!effect)
		{
			continue;
		}

		uintptr_t thisNumber = (uintptr_t)(effect.get());
		std::string thisString = std::to_string(thisNumber);
		ImGui::PushID(thisString.c_str());

		std::string headerLabel = "";

		headerLabel = effect->GetEffectTypeName().data();

		// removes "Effect" suffix from name if it exists
		if (headerLabel.size() >= 6 && headerLabel.compare(headerLabel.size() - 6, 6, "Effect") == 0)
		{
			headerLabel = headerLabel.substr(0, headerLabel.size() - 6);
		}

		bool isOpen = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::BeginPopupContextItem("popup"))
		{
			if (ImGui::MenuItem("Remove Effect"))
			{
				m_currentBus->RemoveEffect(effect.get());
			}
			ImGui::EndPopup();
		}

		if (isOpen)
		{
			effect->RenderUI();
		}

		ImGui::PopID();
	}

	ImGui::Dummy(ImVec2(0, 18));

	ImGui::Indent(buttonMargin);

	if (ImGui::Button(ICON_LC_SPARKLES " Add Effect " ICON_LC_SPARKLES, ImVec2(buttonWidth - 2.0f * buttonMargin, buttonHeight)))
	{
		OpenEffectList();
	}

	std::string selectedComponentName = "";
	if (ShowEffectList(&selectedComponentName))
	{
		//auto newEffect = Droplet::Audio::EffectFactory::CreateEffect(selectedComponentName);
		m_currentBus->AttachEffect(selectedComponentName);
	}


	ImGui::PopID();
}
