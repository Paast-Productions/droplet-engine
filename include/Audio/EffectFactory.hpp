#pragma once
#include <memory>
#include <string>

namespace Droplet::Audio
{
	class Effect;

	class EffectFactory
	{
	public:

		[[nodiscard]] static std::unique_ptr<Effect> CreateEffect(const std::string &p_name);
	};
}