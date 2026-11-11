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

	protected:
		void StartImpl() override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	    GUID m_soundGuid = C_INVALID_GUID;
	    std::string m_soundName;
	    ResourceHandle<SoundResource> m_resourceHandle;
	    
	    dalia::PlaybackHandle m_daliaPlaybackHandle = dalia::INVALID_PLAYBACK_HANDLE;
		float m_volume = 0;
		bool m_looping = false;
		
		std::string m_busName;
		Audio::AudioMixer *m_mixer = nullptr;

		dalia::AttenuationCurve m_attCurve;
	};
}