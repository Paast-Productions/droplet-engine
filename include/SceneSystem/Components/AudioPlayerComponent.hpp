#pragma once

#include <SceneSystem/Component.hpp>

#include <Audio/AudioSystem.hpp>

namespace Droplet::Scene
{
	class AudioPlayerComponent : public Component
	{
	public:

		AudioPlayerComponent();
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioPlayerComponent"; }

		const std::string GetName() { return m_audioName; }
	protected:

		void StartImpl() override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:

		bool m_looping = false;
		float m_pitch = 1.0f;
		float m_pan = 0.5f;
		float m_volume = 1.0f;

		Audio::AudioMixer *m_mixer = nullptr;
		std::string m_audioBusName;

		dalia::PlaybackHandle m_playback;
		//ResourceHandle<SoundResource> m_handle
		std::string m_audioName = "Gupta"; // HACK Just for us to find the emitter in the script


	};
}