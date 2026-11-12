#pragma once
#include <SceneSystem/Component.hpp>
#include <Audio/AudioSystem.hpp>
#include <Resource/Types/SoundResource.hpp>

#include "StaticResourceHandle.hpp"

namespace Droplet::Scene 
{
	
	/// @brief 3D Audio player.
	class AudioEmitterComponent : public Component
	{
	public:
		AudioEmitterComponent();
	    ~AudioEmitterComponent() override;
	    
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioEmitterComponent"; }
	    [[nodiscard]] const std::string& GetName() const { return m_soundName; }
	    
	    // --- Playback Controls ---
	    void Play();
	    void Pause();
	    void Stop();
	    [[nodiscard]] bool IsPlaying() const;
	    
	    void SetVolumeDb(float p_volume);
	    void SetPitch(float p_pitch);
	    void SetLooping(bool p_looping);
	    void SetMinMaxDistance(float p_min, float p_max);
	    void SetAttenuationCurve(dalia::AttenuationCurve p_curve);
	    void SetBusName(const std::string& p_busName);
	    
	    // --- Parameter Getters ---
	    [[nodiscard]] Audio::PlaybackID GetPlaybackID() const { return m_playbackId; }
	    [[nodiscard]] float GetVolumeDb() const { return m_volumeDb; }
	    [[nodiscard]] float GetPitch() const { return m_pitch; }
	    [[nodiscard]] bool IsLooping() const { return m_looping; }
	    [[nodiscard]] float GetMinDistance() const { return m_minDistance; }
	    [[nodiscard]] float GetMaxDistance() const { return m_maxDistance; }
	    [[nodiscard]] dalia::AttenuationCurve GetAttenuationCurve() const { return m_attenuationCurve; }
	    [[nodiscard]] const std::string& GetBusName() const { return m_busName; }
	    
	    [[nodiscard]] bool IsDirty() const { return m_isDirty;}
	    void ClearDirtyFlag() { m_isDirty = false; }
	    
	    // --- Spatial Getters ---
	    [[nodiscard]] glm::vec3 GetWorldPosition() const;
	    [[nodiscard]] glm::vec3 GetVelocity() const;
	    
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
	    std::string m_soundName = "Gupta";
	    StaticResourceHandle<SoundResource> m_resourceHandle;
	    
	    Audio::PlaybackID m_playbackId = Audio::C_INVALID_PLAYBACK_ID;
	    Audio::PlaybackState m_playbackState = Audio::PlaybackState::Playing;
	    bool m_isDirty = false;
	    
		float m_volumeDb = 0.0f;
		float m_pitch = 1.0f;
	    bool m_looping = false;
	    
        dalia::AttenuationCurve m_attenuationCurve = dalia::AttenuationCurve::InverseSquare;
	    float m_minDistance = 1.0f;
		float m_maxDistance = 100.0f;
	    bool m_doppler = false;
	    
	    std::string m_busName = "Master";
	};
}
