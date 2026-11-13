#include <Audio/Effects/HighPassEffect.hpp>
#include <Audio/AudioSystem.hpp>
void Droplet::Audio::HighPassEffect::StartImpl()
{
	m_params.frequency = m_frequency;
	m_params.resonance = m_resonance;
	m_params.type = dalia::BiquadParams::Type::HighPass;
	AudioSystem::GetInstance().CreateEffect(GetEffectHandle(), m_params);
}

void Droplet::Audio::HighPassEffect::RenderUIImpl()
{
	if (ImGui::DragFloat(ICON_LC_AUDIO_LINES " Resonance ", &m_resonance, 0.001f, 0.0f, 1.0f, "%.3f"))
	{
		m_params.resonance = m_resonance;
		if (!AudioSystem::GetInstance().ChangeEffectSetting(GetEffectHandle(), m_params))
		{
			throw std::runtime_error("Could not change effect settings for: " + GetEffectTypeName());
		}
	}

	if (ImGui::DragFloat(ICON_LC_AUDIO_WAVEFORM " Frequency ", &m_frequency, 1.0f, 0.0f, 5000.0f, "%.0f"))
	{
		m_params.frequency = m_frequency;
		if (!AudioSystem::GetInstance().ChangeEffectSetting(GetEffectHandle(), m_params))
		{
			throw std::runtime_error("Could not change effect settings for: " + GetEffectTypeName());

		}
	}
}

nlohmann::json Droplet::Audio::HighPassEffect::SerializeImpl()
{
	nlohmann::json effectJson;

	effectJson["resonance"] = m_resonance;
	effectJson["frequency"] = m_frequency;


	return effectJson;
}

void Droplet::Audio::HighPassEffect::DeserializeImpl(nlohmann::json p_json)
{
	if (p_json.contains("resonance"))
	{
		m_resonance = p_json["resonance"];
	}
	if (p_json.contains("frequency"))
	{
		m_frequency = p_json["frequency"];
	}

	//HACK 
	StartImpl();
}
