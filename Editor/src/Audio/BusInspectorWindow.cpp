#include "BusInspectorWindow.hpp"
#include <Audio/EffectFactory.hpp>
#include <EffectList.hpp>
#include <Audio/Effect.hpp>
#include <Audio/AudioBus.hpp>
#include <Editor/DragDropFields.hpp>
#include <Audio/AudioMixer.hpp>

void Droplet::Editor::BusInspectorWindow::SetCurrentBus(Audio::AudioBus *p_bus)
{
	m_currentBus = p_bus;
}

void Droplet::Editor::BusInspectorWindow::RenderEffectSlotDropTarget(Audio::AudioBus&p_bus, uint32_t p_slot)
{
	ImGui::PushID(p_slot);

	const std::string label = "Slot " + std::to_string(p_slot + 1);
	ImGui::Button(label.c_str(), ImVec2(120.0f, 40.0f));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PAYLOAD_EFFECT.data()))
		{
			if (payload->DataSize == sizeof(EffectDragDropData))
			{
				EffectDragDropData dragged;
				std::memcpy(&dragged, payload->Data, sizeof(dragged));

				if (payload->IsDelivery())
				{
					p_bus.MoveEffect(dragged.slot, p_slot);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::PopID();
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
	float prev = volume;
	if (ImGui::SliderFloat(ICON_LC_BUS ICON_LC_VOLUME_2 " Bus Volume", &volume, Audio::C_VOLUME_DECIBEL_MIN, 
	    Audio::C_VOLUME_DECIBEL_MAX, "%.2f"))
	{
		m_currentBus->SetVolume(volume);
	}

	const float slotPanelWidth = 130.0f;
	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	ImVec2 available = ImGui::GetContentRegionAvail();

	ImGui::BeginChild(ICON_LC_SETTINGS_2 " Effect Controls ", ImVec2(available.x - slotPanelWidth - spacing, 0.0f), ImGuiChildFlags_None);

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

		if (ImGui::BeginDragDropSource())
		{
			EffectDragDropData payload{ effect->GetSlot() };
			ImGui::SetDragDropPayload(PAYLOAD_EFFECT.data(), &payload, sizeof(payload));
			ImGui::TextUnformatted(headerLabel.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginPopupContextItem("popup"))
		{
			if (ImGui::MenuItem("Remove Effect"))
			{
				m_currentBus->RemoveEffect(effect.get());
				isOpen = false;
			}
			ImGui::EndPopup();
		}

		if (isOpen)
		{
			effect->RenderUI();
		}

		ImGui::PopID();
	}

	const float buttonWidth = ImGui::GetContentRegionAvail().x;
	const float buttonHeight = ImGui::GetTextLineHeight() * 1.5f;
	const float buttonMargin = buttonWidth * 0.2f;

	ImGui::Dummy(ImVec2(0, 18));

	ImGui::Indent(buttonMargin);
	//TODO Make this button only show when we don't have a full array of effects
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

	ImGui::EndChild();

	//Right Side Effectslots
	ImGui::SameLine();
	{
		ImGui::BeginChild("EffectSlots", ImVec2(slotPanelWidth, 0.0f), ImGuiChildFlags_None);

		ImGui::TextUnformatted(ICON_LC_BETWEEN_HORIZONAL_START " Effect Slots " );
		ImGui::Separator();

		for (uint32_t slot = 0; slot < Audio::AudioBus::MAX_EFFECTS; ++slot)
		{
			RenderEffectSlotDropTarget(*m_currentBus, slot);
			ImGui::Dummy(ImVec2(0.0, 5.0f));
		}
		ImGui::EndChild();
	}

	ImGui::PopID();
}
