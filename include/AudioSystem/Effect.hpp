#pragma once

namespace Droplet::Audio
{
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

	protected:


	private:

	};
}