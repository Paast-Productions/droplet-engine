#pragma once
#include "Audio/AudioMixer.hpp"
#include "Resource/Types/SoundResource.hpp"
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
	using PlaybackID = uint32_t;
	constexpr PlaybackID C_INVALID_PLAYBACK_ID = 0;
	
	using ListenerID = uint32_t;
	constexpr ListenerID C_INVALID_LISTENER_ID = UINT32_MAX;
	
	constexpr uint32_t C_EDITOR_LISTENER_INDEX = 0;
	
	constexpr float C_VOLUME_DECIBEL_DEFAULT = 0.0f;
	constexpr float C_VOLUME_DECIBEL_MIN = -60.0f;
	constexpr float C_VOLUME_DECIBEL_MAX = 12.0f;
	
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
		
		AudioMixer& GetMixer() { return m_mixer; }
		
		// --- Editor ---
		void SetEditorListenerActive(bool p_active);
		void UpdateEditorListener3DAttributes(const glm::vec3& p_pos, const glm::vec3& p_fwd, 
			const glm::vec3& p_up, const glm::vec3& p_vel);
		
		// --- Effects ---
		bool CreateEffect(dalia::EffectHandle &p_handle, dalia::BiquadParams& p_params);
		bool AttachEffect(dalia::EffectHandle &p_effect, const std::string& p_busname, uint32_t p_slot);
		bool DetachEffect(dalia::EffectHandle &p_effect);
		bool DestroyEffect(dalia::EffectHandle &p_effect);
		bool ChangeEffectSetting(dalia::EffectHandle &p_effect, dalia::BiquadParams &p_params);

	private:
		AudioSystem() = default;
		~AudioSystem() = default;
		
		void ProcessFinishedPlaybacks();
		void RegisterEffects();

		std::unique_ptr<dalia::Engine> m_daliaEngine;
		AudioMixer m_mixer;
		
		// Playback Management
		PlaybackID m_nextPlaybackId = 1;
		std::unordered_map<PlaybackID, dalia::PlaybackHandle> m_activePlaybacks;
		std::vector<PlaybackID> m_finishedPlaybacks;
		
		std::vector<Scene::AudioPlayerComponent*> m_registeredPlayers;
		std::vector<Scene::AudioEmitterComponent*> m_registeredEmitters;
		
		std::vector<Scene::AudioListenerComponent*> m_registeredListeners;
		bool m_listenerSlots[4] = { false, false, false, false };
	};
}
