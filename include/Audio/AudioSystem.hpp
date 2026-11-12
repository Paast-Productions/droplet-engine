#pragma once
#include "AudioMixer.hpp"
#include "Resource/Types/SoundResource.hpp"

#include <unordered_map>
#include <functional>
#include <vector>
#include <filesystem>

namespace Droplet::Scene
{
    class AudioPlayerComponent;
    class AudioEmitterComponent;
    class AudioListenerComponent;
}

namespace Droplet::Audio
{
    using PlaybackID = uint32_t;
    constexpr PlaybackID C_INVALID_PLAYBACK_ID = 0;
    
    class dalia::Engine;
    class dalia::PlaybackHandle;
    
    // TODO: Do we need this?
    enum class PlaybackState : uint8_t
    {
        Playing = 0,
        Paused = 1, 
        Inactive,
    };
    
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
	    void Update();
	    
	    std::unique_ptr<SoundResource> LoadSound(const std::filesystem::path &p_path, SoundType p_type, 
	         std::function<void(bool)> p_onLoad);
	    void UnloadSound(SoundResource* sound);
	    
	    // Component registration
	    void RegisterPlayer(Scene::AudioPlayerComponent* player);
	    void UnregisterPlayer(Scene::AudioPlayerComponent* player);
	    
	    void RegisterEmitter(Scene::AudioEmitterComponent* emitter);
	    void UnregisterEmitter(Scene::AudioEmitterComponent* emitter);
	    
	    void RegisterListener(Scene::AudioListenerComponent* listener);
	    void UnregisterListener(Scene::AudioListenerComponent* listener);
	    
	    PlaybackID Play(Scene::AudioPlayerComponent* player, SoundResource* sound);
	    PlaybackID Play3D(Scene::AudioEmitterComponent* emitter, SoundResource* sound);
	    void Pause(PlaybackID id);
	    void Stop(PlaybackID id);
	    void SetVolume(PlaybackID id, float volume);
	    
	    AudioMixer& GetMixer() { return m_mixer; }

	private:
	    AudioSystem() = default;
	    ~AudioSystem() = default;
	    
	    void ProcessFinishedPlaybacks();

		std::unique_ptr<dalia::Engine> m_daliaEngine;
		AudioMixer m_mixer;
	    
	    // Playback Management
	    PlaybackID m_nextPlaybackId = 1;
	    std::unordered_map<PlaybackID, dalia::PlaybackHandle> m_activePlaybacks;
	    std::vector<PlaybackID> m_finishedPlaybacks;
	    
	    std::vector<Scene::AudioPlayerComponent*> m_activePlayers;
	    std::vector<Scene::AudioEmitterComponent*> m_activeEmitters;
	    
	    std::vector<Scene::AudioListenerComponent*> m_activeListeners;
	    bool m_listenerSlots[4] = { false, false, false, false };
	};
}
