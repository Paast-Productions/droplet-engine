#pragma once
#include <SceneSystem/Component.hpp>
#include <AudioSystem/AudioSystem.hpp>
#include <Resource/ResourceHandle.hpp>
namespace Droplet::Scene 
{
	
	/// @brief Right now just outline class to start work with later
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
		float m_volume = 0.0f;
		float m_pitch = 0.0f;
		bool m_looping = false;
		
		std::string m_audioBusName;
		dalia::PlaybackHandle m_playHandle;
		//ResourceHandle<SoundResource> m_handle
		std::string m_displayName ="";
		Audio::AudioMixer *m_mixer = nullptr;

		dalia::AttenuationCurve m_attCurve;

		float m_maxDistance = 0.0f;
		float m_minDistance = 1.0f;
	};
}