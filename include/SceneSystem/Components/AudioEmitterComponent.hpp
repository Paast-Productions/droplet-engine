#pragma once
#include <SceneSystem/Component.hpp>
#include <Audio/AudioSystem.hpp>
#include <Resource/Types/SoundResource.hpp>

#include "Resource/StaticResourceHandle.hpp"

namespace Droplet::Scene 
{
	
	/// @brief Spatial audio player.
	class AudioEmitterComponent : public Component
	{
	public:
	    ~AudioEmitterComponent() override;
	    
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioEmitterComponent"; }
	    [[nodiscard]] const std::string& GetName() const { return m_soundName; }
	    
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
	    void SetPitch(float p_pitch);
	    void SetLooping(bool p_looping);
	    void SetMinMaxDistance(float p_min, float p_max);
	    void SetAttenuationCurve(dalia::AttenuationCurve p_curve);
	    
	    // --- Parameter Getters ---
	    [[nodiscard]] Audio::PlaybackID GetPlaybackID() const { return m_playbackId; }
	    [[nodiscard]] double GetStartTimeSeconds() const { return m_startTimeSeconds; }
	    [[nodiscard]] float GetVolumeDb() const { return m_volumeDb; }
	    [[nodiscard]] float GetPitch() const { return m_pitch; }
	    [[nodiscard]] bool IsLooping() const { return m_looping; }
	    [[nodiscard]] float GetMinDistance() const { return m_minDistance; }
	    [[nodiscard]] float GetMaxDistance() const { return m_maxDistance; }
	    [[nodiscard]] dalia::AttenuationCurve GetAttenuationCurve() const { return m_attenuationCurve; }
		
		// --- Spatial Getters ---
		[[nodiscard]] glm::vec3 GetWorldPosition() const;
		[[nodiscard]] glm::vec3 GetVelocity() const;
		
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
		float m_pitch = Audio::C_PITCH_DEFAULT;
	    bool m_looping = false;
	    
        dalia::AttenuationCurve m_attenuationCurve = dalia::AttenuationCurve::InverseSquare;
	    float m_minDistance = Audio::C_MIN_DISTANCE_DEFAULT;
		float m_maxDistance = Audio::C_MAX_DISTANCE_DEFAULT;
	    bool m_doppler = false;
	    
	    std::string m_busName = "Master";
		bool m_routingChanged = false;
	};
}
