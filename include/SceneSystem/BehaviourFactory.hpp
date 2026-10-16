#pragma once
#include <memory>
#include <string>

namespace Droplet::Scene
{
	// forward declaration
	class Behaviour;

	// TODO: Hide the behaviour constructor and add this class as a friend class, such that this class is the only way to create behaviours.
	class BehaviourFactory
	{
	public:
		/// @brief Creates a new behaviour instance based on the provided name.
		/// @param name The name of the behaviour type to create.
		/// @return A shared pointer to the newly created behaviour.
		/// @throws std::runtime_error If the behaviour type is not registered in the BehaviourRegistry.
		[[nodiscard]] static std::shared_ptr<Behaviour> CreateBehaviour(const std::string &name);
	};
}