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
	    
	    void PlaySound();
	    
		AudioMixer &GetMixer();

		void TestPrint();
	

	private:
	    AudioSystem() = default;
	    ~AudioSystem() = default;

		dalia::Engine m_daliaEngine;
		AudioMixer m_mixer;
	};
}
