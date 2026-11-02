#pragma once
#include <json/json.hpp>

namespace Droplet::Audio
{
	//forward decleration
	class AudioBus;

	using AudioBusID = uint64_t;

	class AudioMixer
	{
	public:
		AudioMixer();
		~AudioMixer() = default;

		AudioBus* AddAudioBus(std::string p_nameAudioBus, AudioBus *p_parent);
		void RemoveAudioBus(std::string p_name);
		
		void SetBusParent(AudioBus *p_childBus, AudioBus *p_parentBus);
	private:

		/// @brief Audiobus ID, ID 0 is reserved for the master bus
		AudioBusID m_nextBusID = 0;
		/// @brief Masterbus, The root of the bus Hierarchy and is the only one not needing a parent
		AudioBus* m_masterBus;
		/// @brief A map of buses and their IDs, Every bus is owned by the Audio mixer and anyone who would ever think something else is something of a master in the topic of being stupid. 
		std::unordered_map<AudioBusID, std::unique_ptr<AudioBus>> m_buses;

	};
}