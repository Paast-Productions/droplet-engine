#include "ComponentFactory.hpp"
#include <ComponentRegistry.hpp>
#include <Component.hpp>

using namespace Droplet::Scene;

std::shared_ptr<Component> ComponentFactory::CreateComponent(const std::string &name)
{
	auto &registry = ComponentRegistry::GetRegistry();

	auto it = registry.find(name);

	if (it == registry.end())
	{
		throw std::runtime_error("Component type not registered: " + name);
	}

	return it->second();
}
