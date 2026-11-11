#include "Audio/AudioSystem.hpp"
using namespace Droplet::Audio;

bool Droplet::Audio::AudioSystem::Initialize()
{
	dalia::EngineConfig config;
	dalia::Result res = m_daliaEngine.Initialize(config);
	if (res != dalia::Result::Ok) return false;
    
	m_mixer.Initialize(m_daliaEngine);
    
    return true;
}

void Droplet::Audio::AudioSystem::Shutdown()
{
	m_daliaEngine.Shutdown();
    // m_mixer.Shutdown(); // Don't we need this?
}

AudioMixer &Droplet::Audio::AudioSystem::GetMixer()
{
	return m_mixer;
}

void Droplet::Audio::AudioSystem::TestPrint()
{
	std::cout << "We are playing sound " << std::endl;
}
