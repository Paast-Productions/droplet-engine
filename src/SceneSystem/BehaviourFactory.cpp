#include "BehaviourFactory.hpp"
#include <BehaviourRegistry.hpp>
#include <Behaviour.hpp>

using namespace Droplet::Scene;

std::shared_ptr<Behaviour> BehaviourFactory::CreateBehaviour(const std::string &name)
{
	auto &registry = BehaviourRegistry::GetRegistry();

	auto it = registry.find(name);

	if (it == registry.end())
	{
		throw std::runtime_error("Behaviour type not registered: " + name);
	}

	return it->second();
}
