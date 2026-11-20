#pragma once

#include "SceneSystem/Component.hpp"
#include "Audio/AudioSystem.hpp"
#include "Resource/StaticResourceHandle.hpp"

#include <string>

namespace Droplet::Scene
{
	
	/// @brief Non-spatial audio player.
	class AudioPlayerComponent : public Component
	{
	public:
	    ~AudioPlayerComponent() override;
	    
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioPlayerComponent"; }
		const std::string GetName() { return m_soundName; }
	    
		// --- State Control ---
	    void Play();
	    void Pause();
	    void Resume();
	    void Stop();
		
		// --- State Getters ---
	    [[nodiscard]] bool IsPlaying() const;
	    
		// --- Parameter Control ---
	    void SetStartTimeSeconds(double p_startTimeSeconds);
	    void SetVolumeDb(float p_volume);
		void SetStereoPan(float p_stereoPan);
	    void SetPitch(float p_pitch);
	    void SetLooping(bool p_looping);
	    
	    // --- Parameter Getters ---
	    [[nodiscard]] Audio::PlaybackID GetPlaybackID() const { return m_playbackId; }
	    [[nodiscard]] double GetStartTimeSeconds() const { return m_startTimeSeconds; }
	    [[nodiscard]] float GetVolumeDb() const { return m_volumeDb; }
	    [[nodiscard]] float GetPitch() const { return m_pitch; }
	    [[nodiscard]] bool IsLooping() const { return m_looping; }
	    [[nodiscard]] float GetStereoPan() const { return m_stereoPan; }
		
		// --- Routing ---
		[[nodiscard]] bool RoutingChanged() const { return m_routingChanged; }
		void ClearRoutingChangedFlag() { m_routingChanged = false; }
		[[nodiscard]] const std::string& GetBusName() const { return m_busName; }
	    
		// --- Dirty Flag ---
		[[nodiscard]] bool IsDirty() const { return m_isDirty;}
		void ClearDirtyFlag() { m_isDirty = false; }
	    
	    // --- Resource ---
	    void SetSound(GUID p_guid);
	    [[nodiscard]] GUID GetSoundGUID() const { return m_soundGuid; }

		// --- System Control ---
	    void OnPlaybackFinished();
		void OnBusInvalidation();
	    
	protected:
		void StartImpl() override;
		void RenderUIImpl() override;
		
		void ActiveChangedImpl(const bool p_active) override;
	    
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	    GUID m_soundGuid = C_INVALID_GUID;
	    std::string m_soundName = "None";
	    StaticResourceHandle<SoundResource> m_resourceHandle;
	    
	    Audio::PlaybackID m_playbackId = Audio::C_INVALID_PLAYBACK_ID;
	    bool m_isDirty = false;
	    
	    double m_startTimeSeconds = 0.0;
	    float m_volumeDb = Audio::C_VOLUME_DECIBEL_DEFAULT;
	    float m_stereoPan = Audio::C_STEREO_PAN_MIDDLE;
	    float m_pitch = Audio::C_PITCH_DEFAULT;
	    bool m_looping = false;
	    
	    std::string m_busName = "Master";
		bool m_routingChanged = false;
	};
}