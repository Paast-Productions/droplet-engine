#pragma once
#include <SceneSystem/Component.hpp>
#include <Audio/AudioSystem.hpp>

namespace Droplet::Scene 
{
	
	/// @brief 3D Audio player.
	class AudioEmitterComponent : public Component
	{
	public:
		AudioEmitterComponent();
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioEmitterComponent"; }

		const std::string GetName() { return m_soundName; }
	    
	    void PlaySound();

	protected:
		void StartImpl() override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	    GUID m_soundGuid = C_INVALID_GUID;
	    std::string m_soundName = "Gupta";
	    ResourceHandle<SoundResource> m_resourceHandle;
	    
	    dalia::PlaybackHandle m_daliaPlaybackHandle = dalia::INVALID_PLAYBACK_HANDLE;
		float m_volume = 0;
		bool m_looping = false;

	    float m_maxDistance = 0.0f;
	    float m_minDistance = 1.0f;
		
		std::string m_busName;
		Audio::AudioMixer *m_mixer = nullptr;

		dalia::AttenuationCurve m_attCurve;

		float m_maxDistance = 100.0f;
		float m_minDistance = 0.0f;
	};
}