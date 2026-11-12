#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace Droplet::Audio
{
	class Effect;
	class AudioMixer;


	/// @brief AudioBuses are the mixing nodes in the mixing hierarchy, 
	/// Audio bus takes multiple input audio streams and mixes them togheter, and ouputs a single combined stream
	/// Buses are handled by both Unique names and Unique IDs, why both? Gupta didn't know thats why
	/// In Theory Unique Ids can be removed and we can search through with names only.
	class AudioBus
	{
	public:
		static constexpr std::size_t MAX_EFFECTS = 4; //Dalia limits the amount of effects on each bus to 4
		/// @brief Bus initilization, setting the name and id for that audiobus
		/// @param p_name Name
		/// @param p_id ID
		explicit AudioBus(const std::string& p_name, std::uint64_t p_id);
		~AudioBus();
		AudioBus() = delete;
		AudioBus(AudioBus &&) = delete;
		AudioBus &operator=(const AudioBus &) = delete;
		AudioBus &operator=(AudioBus &&) = delete;

		/// @brief Handles the activation state of the audiobus
		/// @param p_active 
		void SetActive(bool p_active);
		/// @brief Returns wheter the bus is active or inactive
		/// @return 
		[[nodiscard]] bool IsActive() const;

		/// @brief Gets the volume for the audio bus
		/// @return 
		float GetVolume() const { return m_volume; }

		/// @brief HACK discuss with arvid how multiple buses volume should interact with each other in the hierarchy
		/// @param p_volume The volume to set
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
		
		/// @brief Routes the bus to be child of this desired bus
		/// @param p_audioBus The Parent bus
		void SetParent(AudioBus* p_audioBus);

		/// @brief Returns the parent of the bus, Only bus that does not have a parent is the master bus
		/// @return Parent bus
		AudioBus* GetParent() const;

		/// @brief Adds a bus to the children vector,
		/// The functions also makes sure that the bus to be childs current parrent relinquish current reference
		/// @param p_audioBus 
		/// @return 
		AudioBus* AddChildBus(AudioBus* p_audioBus);

		/// @brief Removes a bus from the child vector
		/// @param p_audioBus The child that is to unchild
		void RemoveChildBus(const AudioBus* p_audioBus);

		/// @brief Searches the bus parent ancestry and look if it is a descendent of a specific bus
		/// Useful for looking if a new bus or bus route would create a cycle
		/// @param p_audioBus The audiobus you are comparing if you are related to
		/// @return Returns true if it is a descendent
		[[nodiscard]] bool IsDecendantOf(const AudioBus* p_audioBus) const;

		/// @brief Gets the children vector
		/// @return 
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
		


		//----------------------------------------
		// Effects
		//----------------------------------------
		
		/// @brief Adds a effect of the specified type to the audiobus.
		Effect *AttachEffect(const std::string &p_effectType);

		Effect *AttachEffect(std::unique_ptr<Effect> p_effect, std::optional<uint32_t> p_effecSlot = std::nullopt);

		void RemoveEffect(Effect *p_effect);
		void RemoveEffect(uint32_t p_slot);

		void MoveEffect(int p_fromSlot, int p_toSlot);

		Effect *GetEffect(int p_slot);

		const std::array<std::unique_ptr<Effect>, MAX_EFFECTS> &GetAllEffects() const { return m_effects; }

	private:

		friend class AudioMixer;
		uint64_t m_id = 0;

		std::string m_name = "";

		bool m_active = false;

		bool m_started = false;

		//Family
		AudioBus* m_parent = nullptr;
		std::vector<AudioBus*> m_children;

		float m_volume = 1.0f;

		//Effects

		std::array<std::unique_ptr<Effect>, MAX_EFFECTS> m_effects;
	};
}