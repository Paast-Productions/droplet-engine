#include "Audio/Effects/ReverbEffect.hpp"
#include "Audio/AudioSystem.hpp"
#include <ImGui/imgui.h>

void Droplet::Audio::ReverbEffect::StartImpl()
{
	AudioSystem::GetInstance().CreateEffect(GetEffectHandle(), m_params);
}


void Droplet::Audio::ReverbEffect::RenderUIImpl()
{
#ifdef _EDITOR
	// Parameters
	bool paramsChanged = false;

	if (ImGui::SliderFloat("Decay Time", &m_params.decayTimeSeconds, 0.1f, 15.0f, "%.2f s")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Room Size", &m_params.roomSize, 0.25f, 1.0f, "%.2f")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Damping Frequency", &m_params.dampingHz, 500.0f, 20000, "%.0f Hz", 
			ImGuiSliderFlags_Logarithmic)) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Pre-Delay", &m_params.preDelayMs, 0.0f, 100.0f, "%.0f ms")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Dry Mix", &m_params.dryMix, 0.0f, 1.0f, "%.2f")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Wet Mix", &m_params.wetMix, 0.0f, 1.0f, "%.2f")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Low Cut Frequency", &m_params.lowCutHz, 20.0f, 500.0f, "%.0f")) {
		paramsChanged = true;
	}

	if (ImGui::SliderFloat("Modulation", &m_params.modulation, 0.0f, 1.0f, "%.2f")) {
		paramsChanged = true;
	}

	if (ImGui::Checkbox("Freeze", &m_params.freeze)) {
		paramsChanged = true;
	}
	
	if (paramsChanged) AudioSystem::GetInstance().ChangeEffectSetting(GetEffectHandle(), m_params);
	
#endif
}

nlohmann::json Droplet::Audio::ReverbEffect::SerializeImpl()
{
	nlohmann::json data;
	data["decayTimeSeconds"] = m_params.decayTimeSeconds;
	data["roomSize"] = m_params.roomSize;
	data["dampingHz"] = m_params.dampingHz;
	data["preDelayMs"] = m_params.preDelayMs;
	data["dryMix"] = m_params.dryMix;
	data["wetMix"] = m_params.wetMix;
	data["lowCutHz"] = m_params.lowCutHz;
	data["modulation"] = m_params.modulation;
	data["freeze"] = m_params.freeze;
	
	return data;
}

void Droplet::Audio::ReverbEffect::DeserializeImpl(nlohmann::json p_json)
{
	if (p_json.contains("decayTimeSeconds")) m_params.decayTimeSeconds	= p_json["decayTimeSeconds"];
	if (p_json.contains("roomSize")) m_params.roomSize					= p_json["roomSize"];
	if (p_json.contains("dampingHz")) m_params.dampingHz				= p_json["dampingHz"];
	if (p_json.contains("preDelayMs")) m_params.preDelayMs				= p_json["preDelayMs"];
	if (p_json.contains("dryMix")) m_params.dryMix						= p_json["dryMix"];
	if (p_json.contains("wetMix")) m_params.wetMix						= p_json["wetMix"];
	if (p_json.contains("lowCutHz")) m_params.lowCutHz					= p_json["lowCutHz"];
	if (p_json.contains("modulation")) m_params.modulation				= p_json["modulation"];
	if (p_json.contains("freeze")) m_params.freeze						= p_json["freeze"];
}
