#pragma once
#include <json/json.hpp>
#include <dalia/dalia.h>
#include <AudioSystem/AudioBus.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
namespace Droplet::Audio
{

	class AudioMixer
	{
	public:
		AudioMixer();

		void Initialize(dalia::Engine& p_dalia);

		AudioBus* AddAudioBus(const std::string& p_nameAudioBus, AudioBus *p_parent);
		void RemoveAudioBus(std::uint64_t p_id);
		
		void SetBusParent(AudioBus *p_childBus, AudioBus *p_parentBus);

		AudioBus *GetMasterBus() const { return m_masterBus; }
		AudioBus *FindAudioBus(std::uint64_t p_id) const;
	private:

		/// @brief Audiobus ID, ID 0 is reserved for the master bus
		std::uint64_t m_nextBusID = 0;
		/// @brief Masterbus, The root of the bus Hierarchy and is the only one not needing a parent
		AudioBus* m_masterBus;
		/// @brief A map of buses and their IDs, Every bus is owned by the Audio mixer and anyone who would ever think something else is something of a master in the topic of being stupid. 
		std::unordered_map<std::uint64_t, std::unique_ptr<AudioBus>> m_buses;

		//dalia::Engine *m_daliaEngine = nullptr;

	};
}