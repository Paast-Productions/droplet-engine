#include "AudioSystem.hpp"
using namespace Droplet::Audio;

Droplet::Audio::AudioSystem::AudioSystem()
{
}

Droplet::Audio::AudioSystem::~AudioSystem()
{
	ShutDown();
}

void Droplet::Audio::AudioSystem::Initialize()
{
	dalia::EngineConfig config;
	dalia::Result res = m_daliaEngine.Initialize(config);
	if (res != dalia::Result::Ok)
	{
		
		return;
	}
	m_mixer.Initialize(m_daliaEngine);
}

void Droplet::Audio::AudioSystem::ShutDown()
{
	m_daliaEngine.Shutdown();
}

AudioMixer &Droplet::Audio::AudioSystem::GetMixer()
{
	return m_mixer;
}

void Droplet::Audio::AudioSystem::PlaySound()
{
	//TODO call dalias play sound
}

void Droplet::Audio::AudioSystem::PauseSound()
{
	//TODO call dalias pause sound
}
