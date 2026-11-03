#pragma once
#include <AudioSystem/Effect.hpp>


namespace Droplet::Audio
{
	/// @brief Maybe reverb maybe not i needed a test effect --Gupta the dumb one
	class ReverbEffect : public Effect
	{
	public:
		ReverbEffect() = default;
		~ReverbEffect() = default;

		[[no_discard]] std::string GetEffectTypeName() const override { return "ReverbEffect"; }
		void Initilize();
	private:

	};


}