#pragma once
#include "Audio/AudioMixer.hpp"
#include "Resource/Types/SoundResource.hpp"
#include "Audio/Constants.hpp"
#include <dalia/dalia.h> // Ideally we don't include this here

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
    enum class PlaybackState : uint8_t
    {
        Playing     = 0,
        Paused      = 1, 
        Inactive    = 2,
    };
    
    class dalia::Engine;
    struct dalia::PlaybackHandle;
    
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
		AudioSystem &operator=(const AudioSystem &) = delete;
		
		bool Initialize();
		void Shutdown();
	    void Update();
	    
	    std::unique_ptr<SoundResource> LoadSound(const std::filesystem::path &p_path, SoundType p_type, 
	         std::function<void(bool)> p_onLoad);
	    void UnloadSound(SoundResource* sound);
	    double GetSoundLenghtInSeconds(SoundResource* sound) const;
	    
	    // Component registration
	    void RegisterPlayerComponent(Scene::AudioPlayerComponent* player);
	    void UnregisterPlayerComponent(Scene::AudioPlayerComponent* player);
	    
	    void RegisterEmitterComponent(Scene::AudioEmitterComponent* emitter);
	    void UnregisterEmitterComponent(Scene::AudioEmitterComponent* emitter);
	    
	    void RegisterListenerComponent(Scene::AudioListenerComponent* listener);
	    void UnregisterListenerComponent(Scene::AudioListenerComponent* listener);
	    void SetListenerActive(ListenerID p_id, bool p_active);
	    
	    PlaybackID Play(Scene::AudioPlayerComponent* player, SoundResource* sound);
	    PlaybackID PlaySpatial(Scene::AudioEmitterComponent* emitter, SoundResource* sound);
	    void Pause(PlaybackID id);
	    void Resume(PlaybackID id);
	    void Stop(PlaybackID id);
	    
	    [[nodiscard]] PlaybackState GetPlaybackState(PlaybackID id) const;
	    
	    AudioMixer& GetMixer() { return m_mixer; }
	    
	    // --- Editor ---
	    void SetMode(bool p_isGameMode);
	    void SetGamePaused(bool p_isPaused);
	    
	    void UpdateEditorListener3DAttributes(const glm::vec3& p_pos, const glm::vec3& p_fwd, const glm::vec3& p_up, 
	        const glm::vec3& p_vel);
	    
	    // --- Effects ---
	    bool CreateEffect(dalia::EffectHandle &p_handle, dalia::BiquadParams& p_params);
	    bool AttachEffect(dalia::EffectHandle &p_effect, const std::string& p_busname, uint32_t p_slot);
	    bool DetachEffect(dalia::EffectHandle &p_effect);
		bool DestroyEffect(dalia::EffectHandle &p_effect);
		bool ChangeEffectSetting(dalia::EffectHandle &p_effect, dalia::BiquadParams &p_params);

	private:
	    struct PlaybackData
	    {
	        dalia::PlaybackHandle handle = dalia::INVALID_PLAYBACK_HANDLE;
	        PlaybackState state = PlaybackState::Inactive;
	    };
	    
	    struct ListenerData
	    {
	        bool taken = false;
	        bool active = false;
	    };
	    
	    AudioSystem() = default;
	    ~AudioSystem() = default;
	    
	    void ProcessFinishedPlaybacks();
	    void RegisterEffects();
	    
	    bool m_isGameMode = false;  // TODO: This has to default to true in an exported game somehow. Maybe we could
									// check for a definition?
	    bool m_isGamePaused = false;

		std::unique_ptr<dalia::Engine> m_daliaEngine;
		AudioMixer m_mixer;
	    
	    // Playback Management
	    std::vector<Scene::AudioPlayerComponent*> m_playerComponents;
	    std::vector<Scene::AudioEmitterComponent*> m_emitterComponents;
	    
	    PlaybackID m_nextPlaybackId = 1;
	    std::unordered_map<PlaybackID, PlaybackData> m_playbacks;
	    std::vector<PlaybackID> m_finishedPlaybacks;
	    
	    // Listener Management
	    std::vector<Scene::AudioListenerComponent*> m_listenerComponents;
	    std::array<ListenerData, C_LISTENERS_MAX> m_listeners;
	};
}
