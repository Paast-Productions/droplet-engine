#include "ComponentFactory.hpp"
#include <ComponentRegistry.hpp>
#include <Component.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;

std::shared_ptr<Component> ComponentFactory::CreateComponent(const std::string &name)
{
	ZoneScoped;
	ZoneText(name.c_str(), name.size());

	auto &registry = ComponentRegistry::GetRegistry();

	auto it = registry.find(name);

	if (it == registry.end())
	{
		throw std::runtime_error("Component type not registered: " + name);
	}

	return it->second();
}
