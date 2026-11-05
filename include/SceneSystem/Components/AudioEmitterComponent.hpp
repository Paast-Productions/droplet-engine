#pragma once
#include <SceneSystem/Component.hpp>
#include <AudioSystem/AudioSystem.hpp>

namespace Droplet::Scene 
{
	/// @brief Right now just outline class to start work with later
	class AudioEmitterComponent : public Component
	{
	public:
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioEmitterComponent"; }

	protected:
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
		float m_volume = 0;
		bool m_looping = false;
		
		std::string m_audioBusName;
		dalia::PlaybackHandle m_playHandle;
		Audio::AudioMixer &m_mixer;
	};
}