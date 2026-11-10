#include "Audio/AudioSystem.hpp"

#include "Allocator.hpp"
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

std::unique_ptr<Droplet::SoundResource> AudioSystem::LoadSound(const std::filesystem::path &p_path, dalia::SoundType p_type,
    std::function<void(bool)> p_onLoad)
{
    dalia::SoundHandle handle = dalia::INVALID_SOUND_HANDLE;
    dalia::Result res = m_daliaEngine.LoadSoundAsync(handle, p_type, p_path.generic_string().c_str(), 
        [&](uint32_t reqId, dalia::Result result)
    {
        if (result == dalia::Result::Ok) p_onLoad(true);
        else p_onLoad(false);
    });
    
    if (res == dalia::Result::Ok) return std::make_unique<SoundResource>(handle);
    return nullptr;
}

AudioMixer &Droplet::Audio::AudioSystem::GetMixer()
{
	return m_mixer;
}
