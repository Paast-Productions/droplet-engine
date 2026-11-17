#include <Audio/Effects/ReverbEffect.hpp>
#include "Audio/AudioSystem.hpp"

void Droplet::Audio::ReverbEffect::Initilize()
{
}

void Droplet::Audio::ReverbEffect::StartImpl()
{
	dalia::BiquadParams lowpassParams;
	lowpassParams.type = dalia::BiquadParams::Type::LowPass;
	lowpassParams.frequency = 800.0f;
	lowpassParams.resonance = 1.0f;

	AudioSystem::GetInstance().CreateEffect(GetEffectHandle(), lowpassParams);
}

void Droplet::Audio::ReverbEffect::RenderUIImpl()
{
	const char *text = ICON_LC_CIRCLE_USER_ROUND " DUMMY " ICON_LC_CIRCLE_USER_ROUND;

	ImGui::SeparatorText(text);
	ImGui::SeparatorText(text);
	ImGui::SeparatorText(text);

}