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

		ImGui::SameLine();
		ImGui::Text(" Effect slot: %u" , GetSlot()+1);

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

void Droplet::Audio::Effect::SetSlot(uint32_t p_slot)
{
	m_effectSlot = p_slot;
}

nlohmann::json Droplet::Audio::Effect::Serialize()
{
	ZoneScoped;
	ZoneText(GetEffectTypeName().data(), GetEffectTypeName().size());

	nlohmann::json effectJson;

	effectJson["type"] = GetEffectTypeName();
	effectJson["active"] = IsActive();
	effectJson["slot"] = GetSlot();
	effectJson["data"] = SerializeImpl();

	return effectJson;
}

void Droplet::Audio::Effect::Deserialize(nlohmann::json p_json)
{
	ZoneScoped;
	ZoneText(GetEffectTypeName().data(), GetEffectTypeName().size());

	if (p_json.contains("active"))
	{
		SetActive(p_json["active"]);
	}
	if (p_json.contains("slot"))
	{
		SetSlot(p_json["slot"]);
	}
	if (p_json.contains("data"))
	{
		nlohmann::json dataJson = p_json["data"];
		DeserializeImpl(dataJson);
	}

}
