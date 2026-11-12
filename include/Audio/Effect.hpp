#pragma once

#include <json/json.hpp>
#include <memory>
#include <string_view>
#include <dalia/dalia.h>

namespace Droplet::Audio
{
	class AudioBus;

	class Effect : public std::enable_shared_from_this<Effect>
	{
	public:

		Effect();
		
		virtual ~Effect() = default;

		Effect(const Effect &) = delete;
		Effect(Effect &&) = delete;
		Effect &operator=(const  Effect &) = delete;
		Effect &operator=(Effect &&) = delete;

		virtual std::string GetEffectTypeName() const = 0;

		[[nodiscard]] AudioBus *GetOwner() const;

		dalia::EffectHandle GetEffectHandle() { return m_effectHandle; }

	protected:

		virtual void StartImpl() {}

		virtual void RenderUIImpl() {}

		[[nodiscard]] virtual nlohmann::json SerializeImpl() { return {}; }

		virtual void DeserializeImpl([[maybe_unused]] const nlohmann::json p_effectJson) {}


	private:

		friend class AudioBus;

		AudioBus *m_owner = nullptr;

		void SetOwner(AudioBus *p_owner);

		dalia::EffectHandle m_effectHandle;

	};
}