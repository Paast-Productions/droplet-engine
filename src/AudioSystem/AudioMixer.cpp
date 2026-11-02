#include "AudioMixer.hpp"
using namespace Droplet::Audio;

Droplet::Audio::AudioMixer::AudioMixer()
{
	const AudioBusID masterID = m_nextBusID++;
	std::unique_ptr<AudioBus> master = std::make_unique<AudioBus>();
	m_masterBus = master.get();
	m_buses.emplace(masterID, std::move(m_masterBus));
}

AudioBus* Droplet::Audio::AudioMixer::AddAudioBus(std::string p_name, AudioBus* p_parent)
{
	if (p_parent == nullptr)
	{
		throw std::invalid_argument("Audio bus parent cannot be nullptr")
	}
	return nullptr;
}

void Droplet::Audio::AudioMixer::RemoveAudioBus(std::string p_name)
{

}

void Droplet::Audio::AudioMixer::SetBusParent(AudioBus *p_childBus, AudioBus *p_parentBus)
{

}
