#pragma once

#include "SceneSystem/Component.hpp"
#include "Audio/AudioSystem.hpp"
#include "Resource/StaticResourceHandle.hpp"

#include <string>

namespace Droplet::Scene
{
	class AudioPlayerComponent : public Component
	{
	public:
		AudioPlayerComponent();
	    ~AudioPlayerComponent() override;
	    
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioPlayerComponent"; }
		const std::string GetName() { return m_soundName; }
	    
	    // --- Playback Controls ---
	    void Play();
	    void Pause();
	    void Resume();
	    void Stop();
	    [[nodiscard]] bool IsPlaying() const;
	    
	    void SetStartTimeSeconds(double p_startTimeSeconds);
	    void SetVolumeDb(float p_volume);
	    void SetPitch(float p_pitch);
	    void SetLooping(bool p_looping);
	    void SetBusName(const std::string& p_busName);
	    
	    // --- Parameter Getters ---
	    [[nodiscard]] Audio::PlaybackID GetPlaybackID() const { return m_playbackId; }
	    [[nodiscard]] double GetStartTimeSeconds() const { return m_startTimeSeconds; }
	    [[nodiscard]] float GetVolumeDb() const { return m_volumeDb; }
	    [[nodiscard]] float GetPitch() const { return m_pitch; }
	    [[nodiscard]] bool IsLooping() const { return m_looping; }
	    [[nodiscard]] bool GetStereoPan() const { return m_stereoPan; }
	    [[nodiscard]] const std::string& GetBusName() const { return m_busName; }
	    
	    [[nodiscard]] bool IsDirty() const { return m_isDirty;}
	    void ClearDirtyFlag() { m_isDirty = false; }
	    
	    // --- Resource ---
	    void SetSoundGUID(GUID p_guid);
	    [[nodiscard]] GUID GetSoundGUID() const { return m_soundGuid; }

	    void OnPlaybackFinished();
	    
	protected:
		void StartImpl() override;
		void RenderUIImpl() override;

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
	};
}