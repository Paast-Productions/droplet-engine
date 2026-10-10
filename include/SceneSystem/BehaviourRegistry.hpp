#pragma once
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <stdexcept>

namespace Droplet::Scene
{
	// forward declaration
	class Behaviour;

	/// @brief Behaviour registry class that manages the registration and retrieval of behaviour types.
	/// This class provides a centralized registry for behaviour types, allowing for dynamic creation of behaviours based on their names.
	/// Additionally, it provides an entrypoint for the game to register custom behaviours such that the engine can create them dynamically.
	class BehaviourRegistry
	{
	public:
		/// @brief Retrieves the registry of behaviour types.
		/// @return A constant reference to the unordered map containing behaviour names and their corresponding construction functions.
		[[nodiscard]] static const std::unordered_map<std::string, std::function<std::shared_ptr<Behaviour>()>> &GetRegistry()
		{
			return Get().registry;
		}

		/// @brief Registers a new behaviour type with the registry.
		/// This function should be called once for every behaviour type, during application initialization.
		/// This ensures that the behaviours are available before scene loading.
		/// @param name The unique name of the behaviour type.
		/// @param constructFunc The function used to create instances of the behaviour.
		static void RegisterBehaviour(const std::string &name, std::function<std::shared_ptr<Behaviour>()> constructFunc)
		{
			BehaviourRegistry &registry = Get();

			// Ensure that the behaviour name is unique and not already registered
			if (registry.registry.find(name) != registry.registry.end())
			{
				throw std::runtime_error("Behaviour with name '" + name + "' is already registered.");
			}

			registry.registry[name] = constructFunc;
		}

	private:
		/// @brief Retrieves the singleton instance of the BehaviourRegistry.
		/// @return A reference to the singleton instance of the BehaviourRegistry.
		[[nodiscard]] static BehaviourRegistry &Get()
		{
			static BehaviourRegistry instance;
			return instance;
		}

		std::unordered_map<std::string, std::function<std::shared_ptr<Behaviour>()>> registry;
	};
}