#pragma once
#include <dalia/dalia.h>
#include "AudioMixer.hpp"


namespace Droplet::Audio
{
	class AudioSystem
	{
	public:
	    /// @brief Retrieves the singleton instance of the audio system.
	    /// @return A reference to the audio system singleton.
	    [[nodiscard]] static AudioSystem &GetInstance()
	    {
	        static AudioSystem instance{};
	        return instance;
	    }

	    AudioSystem(const AudioSystem &) = delete;
	    ResourceManager &operator=(const ResourceManager &) = delete;
	    
	    bool Initialize();
		void Shutdown();
	    
	    std::unique_ptr<SoundResource> LoadSound(const std::filesystem::path &p_path, SoundType p_type, 
	         std::function<void(bool)> p_onLoad);
	    
		void PopulatePlayBack(dalia::PlaybackHandle p_playback, dalia::SoundHandle p_soundHandle);

		void ChangePlayBackVolume(dalia::PlaybackHandle p_playback, float p_volumeChange);
		void SetLooping(dalia::PlaybackHandle p_playback, bool p_looping);

	    void PlaySound(dalia::PlaybackHandle p_playback);
		void PauseSound(dalia::PlaybackHandle p_playback);
		void StopSound(dalia::PlaybackHandle p_playback);

		void RoutePlaybackToBus(dalia::PlaybackHandle, std::string p_busName);

		AudioMixer &GetMixer();

		void TestPrint();
	

	private:
	    AudioSystem() = default;
	    ~AudioSystem() = default;

		void RegisterEffects();

		dalia::Engine m_daliaEngine;
		AudioMixer m_mixer;
	};
}
