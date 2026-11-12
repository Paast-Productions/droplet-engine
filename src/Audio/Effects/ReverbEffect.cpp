#include <Audio/Effects/ReverbEffect.hpp>

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
