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

		float GetVolume() const { return m_volume; }
		void SetVolume(float p_volume);

		AudioBusID GetID() const { return m_id; }
		void SetID(AudioBusID p_id) { m_id = p_id; }

		//---------------------------------------
		// Name
		//---------------------------------------

		void SetName(std::string p_name);
		std::string GetName() const { return m_name; }


		// --------------------------------------
		// Children and parent AudioBuses
		//---------------------------------------
		void SetParent(AudioBus* p_audioBus);
		AudioBus* GetParent() const;

		AudioBus* AddChildBus(AudioBus* p_audioBus);

		void RemoveChildBus(const AudioBus* p_audioBus);

		[[nodiscard]] bool IsDecendantOf(const AudioBus* p_audioBus) const;

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

		friend class AudioMixer;
		AudioBusID m_id;

		std::string m_name;

		bool m_active = false;

		//Family
		AudioBus* m_parent = nullptr;
		std::vector<AudioBus*> m_children;

		float m_volume;
	};
}