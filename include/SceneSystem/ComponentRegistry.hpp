#pragma once
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <stdexcept>

namespace Droplet::Scene
{
	// forward declaration
	class Component;

	/// @brief Component registry class that manages the registration and retrieval of component types.
	/// This class provides a centralized registry for component types, allowing for dynamic creation of components based on their names.
	/// Additionally, it provides an entrypoint for the game to register custom components such that the engine can create them dynamically.
	class ComponentRegistry
	{
	public:
		/// @brief Retrieves the registry of component types.
		/// @return A constant reference to the unordered map containing component names and their corresponding construction functions.
		static const std::unordered_map<std::string, std::function<std::shared_ptr<Component>()>> &GetRegistry()
		{
			return Get().registry;
		}

		/// @brief Registers a new component type with the registry.
		/// This function should be called once for every component type, during application initialization.
		/// This ensures that the components are available before scene loading.
		/// @param name The unique name of the component type.
		/// @param constructFunc The function used to create instances of the component.
		static void RegisterComponent(const std::string &name, std::function<std::shared_ptr<Component>()> constructFunc)
		{
			ComponentRegistry &registry = Get();

			// Ensure that the component name is unique and not already registered
			if (registry.registry.find(name) != registry.registry.end())
			{
				throw std::runtime_error("Component with name '" + name + "' is already registered.");
			}

			registry.registry[name] = constructFunc;
		}

	private:
		/// @brief Retrieves the singleton instance of the ComponentRegistry.
		/// @return A reference to the singleton instance of the ComponentRegistry.
		static ComponentRegistry &Get()
		{
			static ComponentRegistry instance;
			return instance;
		}

		std::unordered_map<std::string, std::function<std::shared_ptr<Component>()>> registry;
	};
}