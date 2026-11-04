#include "MixerWindow.hpp"
#include <AudioSystem/AudioBus.hpp>
Droplet::Editor::MixerWindow::MixerWindow(EditorContext *p_editorContext) : m_editorContext(p_editorContext)
{
}
using namespace Droplet;

void Droplet::Editor::MixerWindow::InitImpl()
{
	SetName("Audio Mixer");
}

void Droplet::Editor::MixerWindow::CloseImpl()
{
}

void Droplet::Editor::MixerWindow::RenderImpl()
{
	Audio::AudioMixer &mixer = m_editorContext->GetAudioSystem().GetMixer();
	AudioMixerEditorState &state = m_editorContext->GetAudioMixerState();

	Audio::AudioBus *selectedBus = nullptr;

	if (state.selectedAudioBusID.has_value())
	{
		selectedBus = mixer.FindAudioBus(state.selectedAudioBusID.value());
		if (selectedBus == nullptr)
		{
			state.selectedAudioBusID.reset();
		}
	}

	if (ImGui::Button("Add Bus"))
	{
		Audio::AudioBus *newBus =
			mixer.AddAudioBus("New Bus", mixer.GetMasterBus());

		if (newBus != nullptr)
		{
			state.selectedAudioBusID = newBus->GetID();
		}
	}

	ImGui::SeparatorText("Bus hierarchy");
	ImGui::BeginChild("BusHierarchy", ImVec2(0.0f, 0.0f), true);
	const ImVec2 available = ImGui::GetContentRegionAvail();
	constexpr float rootDropWidth = 8.0f;

	//Fixed stripe

	ImGui::BeginChild("RootDropStripe", ImVec2(rootDropWidth, available.y), false);
	ImGui::InvisibleButton("RootDropTarget", ImVec2(rootDropWidth, available.y));

	const ImVec2 stripeMin = ImGui::GetItemRectMin();
	const ImVec2 stripeMax = ImGui::GetItemRectMax();

	ImGui::GetWindowDrawList()->AddRectFilled(stripeMin, stripeMax, IM_COL32(90, 130, 180, 100));
	HandleBusDropTarget(mixer.GetMasterBus());
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("BusTree", ImVec2(0.0f, available.y), false);

	DrawBus(mixer.GetMasterBus());

	const ImVec2 remaining = ImGui::GetContentRegionAvail();

	if (remaining.y > 0.0f)
	{
		ImGui::InvisibleButton("BottomRootDropTarget", ImVec2(remaining.x, remaining.y));
		HandleBusDropTarget(mixer.GetMasterBus());
	}

	ImGui::EndChild();
	ImGui::EndChild();

	if (m_pendingReparent.has_value())
	{
		const PendingReparent &operation = m_pendingReparent.value();

		Audio::AudioBus *bus = mixer.FindAudioBus(operation.busID);
		Audio::AudioBus *parent = mixer.FindAudioBus(operation.newParentID);

		if (bus != nullptr && parent != nullptr)
		{
			mixer.SetBusParent(bus, parent);
		}
		m_pendingReparent.reset();
	}
	
	if (m_pendingRemoveBusID.has_value())
	{
		const uint64_t operation = m_pendingRemoveBusID.value();
		mixer.RemoveAudioBus(operation);

		m_pendingRemoveBusID.reset();
	}
}

void Droplet::Editor::MixerWindow::RenderToolbar()
{
}

void Droplet::Editor::MixerWindow::DrawBus(Audio::AudioBus *p_bus)
{
	if (p_bus == nullptr)
	{
		return;
	}

	Audio::AudioMixer &mixer = m_editorContext->GetAudioSystem().GetMixer();
	AudioMixerEditorState &state = m_editorContext->GetAudioMixerState();

	const std::uint64_t busID = p_bus->GetID();
	const bool hasChildren = !p_bus->GetChildren().empty();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (!hasChildren)
	{
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	if (state.selectedAudioBusID.has_value() && state.selectedAudioBusID.value() == busID)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	const std::string idString = std::to_string(busID);
	ImGui::PushID(idString.c_str());

	const bool nodeOpen = ImGui::TreeNodeEx(p_bus->GetName().c_str(), flags);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		state.selectedAudioBusID = busID;
	}

	if (ImGui::BeginPopupContextItem("BusContextMenu"))
	{
		if (ImGui::MenuItem("Add Child Bus"))
		{
			Audio::AudioBus *newBus = mixer.AddAudioBus("New bus", p_bus);

			if (newBus != nullptr)
			{
				state.selectedAudioBusID = newBus->GetID();
			}
		}

		if (ImGui::MenuItem("Remove Bus"))
		{
			if (p_bus != mixer.GetMasterBus())
			{
				m_pendingRemoveBusID = p_bus->GetID();
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource() && p_bus != mixer.GetMasterBus())
	{
		Audio::AudioBus *bus = p_bus;
		ImGui::SetDragDropPayload("AUDIO_BUS", &busID, sizeof(busID));

		ImGui::Text("%s", p_bus->GetName().c_str());

		ImGui::EndDragDropSource();
	}

	HandleBusDropTarget(p_bus);

	if (nodeOpen && hasChildren)
	{
		for (Audio::AudioBus *child : p_bus->GetChildren())
		{
			DrawBus(child);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Droplet::Editor::MixerWindow::HandleBusDropTarget(Audio::AudioBus *p_parent)
{
	if (p_parent == nullptr)
	{
		return;
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("AUDIO_BUS");

		if (payload != nullptr && payload->IsDelivery() && payload->DataSize == sizeof(std::uint64_t))
		{
			std::uint64_t draggedBusID = 0;
			std::memcpy(&draggedBusID, payload->Data, sizeof(draggedBusID));

			Audio::AudioMixer &mixer = m_editorContext->GetAudioSystem().GetMixer();
			Audio::AudioBus *draggedBus = mixer.FindAudioBus(draggedBusID);

			if (draggedBus != nullptr && draggedBus != mixer.GetMasterBus() && !p_parent->IsDecendantOf(draggedBus))
			{
				m_pendingReparent = PendingReparent{ draggedBusID, p_parent->GetID() };
			}
		}
		ImGui::EndDragDropTarget();
	}
}
