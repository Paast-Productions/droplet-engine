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

void Droplet::Audio::AudioSystem::PopulatePlayBack(dalia::PlaybackHandle p_playback, dalia::SoundHandle p_soundHandle)
{
    dalia::Result res; 
    res = m_daliaEngine.CreatePlayback(p_playback, p_soundHandle);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not populate playback");
    }
}

void Droplet::Audio::AudioSystem::ChangePlayBackVolume(dalia::PlaybackHandle p_playback, float p_volumeChange)
{
    dalia::Result res;
    res = m_daliaEngine.SetPlaybackVolumeDb(p_playback, p_volumeChange);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not change volume on playback");
    }
}

void Droplet::Audio::AudioSystem::SetLooping(dalia::PlaybackHandle p_playback, bool p_looping)
{
    dalia::Result res;
    res = m_daliaEngine.SetPlaybackLooping(p_playback, p_looping);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not change playback too: " + p_looping);
    }
}

void Droplet::Audio::AudioSystem::PlaySound(dalia::PlaybackHandle p_playback)
{
    dalia::Result res;
    res = m_daliaEngine.PlayPlayback(p_playback);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not play playback");
    }
}

void Droplet::Audio::AudioSystem::PauseSound(dalia::PlaybackHandle p_playback)
{
    dalia::Result res;
    res = m_daliaEngine.PausePlayback(p_playback);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not pause playback");
    }
}

void Droplet::Audio::AudioSystem::StopSound(dalia::PlaybackHandle p_playback)
{
    dalia::Result res;
    res = m_daliaEngine.StopPlayback(p_playback);
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not stop playback");
    }
}

void Droplet::Audio::AudioSystem::RoutePlaybackToBus(dalia::PlaybackHandle p_playback, std::string p_busName)
{
    if (!m_mixer.HasAudioBusWithName(p_busName))
    {
        return;
    }
    dalia::Result res;
    res = m_daliaEngine.RoutePlayback(p_playback, p_busName.c_str());
    if (res != dalia::Result::Ok)
    {
        throw std::runtime_error("Could not route playback to bus:" + p_busName);
    }
}

AudioMixer &Droplet::Audio::AudioSystem::GetMixer()
{
	return m_mixer;
}

void Droplet::Audio::AudioSystem::TestPrint()
{
	std::cout << "We are playing sound " << std::endl;
}
