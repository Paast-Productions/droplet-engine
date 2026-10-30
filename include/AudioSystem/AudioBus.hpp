#pragma once
#include "AudioMixer.hpp"


namespace Droplet::Audio
{
	class AudioBus
	{
	public:
		AudioBus() = default;
		~AudioBus() = default;

		AudioBus(AudioBus &&) = delete;
		AudioBus &operator=(const AudioBus &) = delete;
		AudioBus &operator=(AudioBus &&) = delete;

		void SetActive(bool p_active);
		[[nodiscard]] bool IsActive() const;
		//---------------------------------------
		// Name
		//---------------------------------------

		void SetName(std::string p_name);
		std::string GetName() const { return m_name; }


		// --------------------------------------
		// Children and parent AudioBuses
		//---------------------------------------
		void SetParent(std::weak_ptr<AudioBus> p_audioBus);
		std::weak_ptr<AudioBus> GetParent() const;

		std::weak_ptr<AudioBus> AddChildBus(std::shared_ptr<AudioBus> p_audioBus);

		void RemoveChildBus(const std::weak_ptr<AudioBus> p_audioBus);

		[[nodiscard]] bool IsDecendantOf(const std::weak_ptr<AudioBus> &p_audioBus) const;

		//---------------------------------------
		// Render UI
		//---------------------------------------
		void RenderUI();



		//---------------------------------------
		// Serialization
		//---------------------------------------

		nlohmann::json SerializeAudioBus();
		void DeserializeAudioBus(nlohmann::json p_json, std::weak_ptr<AudioBus> p_parentBus);
		

	private:
		std::string m_name;

		bool m_active = false;

		std::weak_ptr<AudioBus> m_parent;
		std::vector<std::shared_ptr<AudioBus>> m_children;

	};
}