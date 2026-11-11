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

void AudioSystem::Update()
{
    m_daliaEngine.Update();
}

std::unique_ptr<Droplet::SoundResource> AudioSystem::LoadSound(const std::filesystem::path &p_path, SoundType p_type,
                                                               std::function<void(bool)> p_onLoad)
{
    dalia::SoundType daliaSoundType;
    if (p_type == SoundType::Resident) daliaSoundType = dalia::SoundType::Resident;
    else daliaSoundType = dalia::SoundType::Stream;
    
    dalia::SoundHandle handle = dalia::INVALID_SOUND_HANDLE;
    dalia::Result res = m_daliaEngine.LoadSoundAsync(handle, daliaSoundType, p_path.generic_string().c_str(), 
        [p_onLoad](uint32_t reqId, dalia::Result result)
    {
        if (p_onLoad)
        {
            if (result == dalia::Result::Ok) p_onLoad(true);
            else p_onLoad(false);
        }
    });
    
    if (res == dalia::Result::Ok) return std::make_unique<SoundResource>(handle);
    return nullptr;
}

void AudioSystem::PlaySound()
{
    // Yep
}

AudioMixer &Droplet::Audio::AudioSystem::GetMixer()
{
	return m_mixer;
}

void Droplet::Audio::AudioSystem::TestPrint()
{
	std::cout << "We are playing sound " << std::endl;
}
