#pragma once
#include <json/json.hpp>
#include <dalia/dalia.h>
#include <Audio/AudioBus.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
namespace Droplet::Audio
{
	/// @brief The audiomixer controls the mixing hierarchy
	/// The audio mixer can add remove and route audiobuses for the required mix
	class AudioMixer
	{
	public:
		AudioMixer();

		/// @brief Initialization gives the mixer the correct dalia instance, It also the masterbus that is the root of the whole hierarchy
		/// Each new bus created is default to be routed to the master bus
		/// 
		/// @param p_dalia 
		void Initialize(dalia::Engine& p_dalia);

		/// @brief Creates and adds a new audiobus, a bus can be directly routed to a specific bus but should always take th master bus as a fall back
		/// To create the audiobus a unique name needs to be submitted and the bus will not be created if a bus with the same name is already in the mixing hierarchy
		/// @param p_nameAudioBus The unique name of the audio bus
		/// @param p_parent The parent the bus should be routed to
		/// @return Returns a pointer to the newly created audiobus
		AudioBus* AddAudioBus(const std::string& p_nameAudioBus, AudioBus *p_parent);

		/// @brief Removes the audio bus, the bus needs to be in the mixing hierarchy, if the masterbus is trying to be removed the system will reject the request of deletion
		/// @param p_id The unique id of the bus that should be removed
		void RemoveAudioBus(std::uint64_t p_id);
		

		/// @brief The routing function for the mixing hierarchy, The function checks if the bus is trying to route to itself, or any other descendents
		/// The function make sure that no cycles appear in the hierarchy
		/// The function also 
		/// @param p_childBus 
		/// @param p_parentBus 
		void SetBusParent(AudioBus *p_childBus, AudioBus *p_parentBus);
		
		/// @brief Iterates through the bus hierarchy to locate a bus with the same name as input
		/// @param p_name The name to search for
		/// @return retuns true if the bushierarchy has a bus with that name, false otherwise
		bool HasAudioBusWithName(const std::string &p_name);

		/// @brief Gets the masterbus, the root of the hierarchy
		/// @return MasterBus
		AudioBus *GetMasterBus() const { return m_masterBus; }

		/// @brief Finds a bus with the specific ID
		/// @param p_id Bus ID
		/// @return The Audiobus with p_id
		AudioBus *FindAudioBus(std::uint64_t p_id) const;
	private:

		/// @brief Audiobus ID, ID 0 is reserved for the master bus
		std::uint64_t m_nextBusID = 0;
		/// @brief Masterbus, The root of the bus Hierarchy and is the only one not needing a parent
		AudioBus* m_masterBus;
		/// @brief A map of buses and their IDs, Every bus is owned by the Audio mixer and anyone who would ever think something else is something of a master in the topic of being stupid. 
		std::unordered_map<std::uint64_t, std::unique_ptr<AudioBus>> m_buses;

		/// @brief reference to dalia
		dalia::Engine *m_daliaEngine = nullptr;

	};
}