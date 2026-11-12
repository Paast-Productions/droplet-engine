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
