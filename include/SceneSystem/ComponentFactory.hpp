#pragma once
#include <memory>
#include <string>

namespace Droplet::Scene
{
	// forward declaration
	class Component;

	// TODO: See if theres a way to hide the component constructor such that this class is the only way to create components.
	class ComponentFactory
	{
	public:
		/// @brief Creates a new component instance based on the provided name.
		/// @param name The name of the component type to create.
		/// @return A shared pointer to the newly created component.
		/// @throws std::runtime_error If the component type is not registered in the ComponentRegistry.
		static std::shared_ptr<Component> CreateComponent(const std::string &name);
	};
}