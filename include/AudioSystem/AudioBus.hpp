#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace Droplet::Audio
{
	class AudioMixer;

	class AudioBus
	{
	public:

		explicit AudioBus(const std::string& p_name, std::uint64_t p_id);
		~AudioBus() = default;
		AudioBus() = delete;
		AudioBus(AudioBus &&) = delete;
		AudioBus &operator=(const AudioBus &) = delete;
		AudioBus &operator=(AudioBus &&) = delete;

		void SetActive(bool p_active);
		[[nodiscard]] bool IsActive() const;

		float GetVolume() const { return m_volume; }
		void SetVolume(float p_volume);

		uint64_t GetID() const { return m_id; }
		void SetID(uint64_t p_id) { m_id = p_id; }

		//---------------------------------------
		// Name
		//---------------------------------------

		void SetName(std::string p_name);
		const std::string &GetName() const { return m_name; }

		// --------------------------------------
		// Children and parent AudioBuses
		//---------------------------------------

		void SetParent(AudioBus* p_audioBus);
		AudioBus* GetParent() const;

		AudioBus* AddChildBus(AudioBus* p_audioBus);

		void RemoveChildBus(const AudioBus* p_audioBus);

		[[nodiscard]] bool IsDecendantOf(const AudioBus* p_audioBus) const;

		const std::vector<AudioBus *>& GetChildren() const;
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
		uint64_t m_id = 0;

		std::string m_name = "";

		bool m_active = false;

		//Family
		AudioBus* m_parent = nullptr;
		std::vector<AudioBus*> m_children;

		float m_volume = 1.0f;
	};
}