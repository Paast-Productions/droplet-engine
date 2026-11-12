#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <stdexcept>

namespace Droplet::Audio
{
	class Effect;

	class EffectRegistry
	{
	public:

		[[nodiscard]] static const std::unordered_map<std::string, std::function<std::unique_ptr<Effect>()>> &GetEffectRegistry()
		{
			return GetInstance().registry;
		}

		static void RegisterEffect(const std::string &name, std::function<std::unique_ptr<Effect>()> constructFunc)
		{
			EffectRegistry &registry = GetInstance();

			if (registry.registry.find(name) != registry.registry.end())
			{
				throw std::runtime_error("Effect with name '" + name + "' is already registrered.");
			}

			registry.registry[name] = constructFunc;
		}

		[[nodiscard]] static bool HasEffect(const std::string &name)
		{
			const auto &registry = GetEffectRegistry();
			return registry.find(name) != registry.end();
		}

		[[nodiscard]] static bool HasEffect(const std::string_view name)
		{
			return HasEffect(std::string(name));
		}

	private:

		[[nodiscard]] static EffectRegistry &GetInstance()
		{
			static EffectRegistry instance;
			return instance;
		}

		std::unordered_map<std::string, std::function<std::unique_ptr<Effect>()>> registry;
	};
}