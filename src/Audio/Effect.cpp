#include "Audio/Effect.hpp"
#include <Audio/AudioBus.hpp>

using namespace Droplet::Audio;

Droplet::Audio::Effect::Effect()
{
}

AudioBus *Droplet::Audio::Effect::GetOwner() const
{
	return nullptr;
}

void Droplet::Audio::Effect::SetOwner(AudioBus *p_owner)
{
	m_owner = p_owner;
}


void Effect::Start()
{
	if ( m_started)
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetEffectTypeName().data(), GetEffectTypeName().size());

	StartImpl();

	m_started = true;
}

void Effect::RenderUI()
{
	ZoneScoped;
	ZoneText(GetEffectTypeName().data(), GetEffectTypeName().size());


	ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
	if (ImGui::BeginChild("EffectUI", ImVec2(0, 0), childFlags))
	{
		bool active = IsActive();
		if (ImGui::Checkbox("Active ", &active))
		{
			SetActive(active);
		}
		ImGui::Dummy(ImVec2(0, 8.0f));
		ImGui::Separator();
		RenderUIImpl();
	}

	ImGui::EndChild();
}

void Droplet::Audio::Effect::SetActive(bool p_active)
{
	m_active = p_active;
}
