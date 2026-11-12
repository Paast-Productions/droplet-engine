#include "EffectFactory.hpp"
#include <EffectRegistry.hpp>
#include <Effect.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Audio;

std::unique_ptr<Effect> Droplet::Audio::EffectFactory::CreateEffect(const std::string &p_name)
{
	ZoneScoped;
	ZoneText(p_name.c_str(), p_name.size());

	auto &registry = EffectRegistry::GetEffectRegistry();

	auto it = registry.find(p_name);

	if (it == registry.end())
	{
		throw std::runtime_error("Effect type not registered: " + p_name);
	}

	return it->second();
}
