#pragma once

#include <Audio/Effect.hpp>

namespace Droplet::Audio
{
	class HighPassEffect : public Effect
	{
	public:
		HighPassEffect() = default;
		~HighPassEffect() = default;

		[[nodiscard]] std::string GetEffectTypeName() const override { return "HighPassEffect"; }

	protected:
		void StartImpl() override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_json) override;

	private:
		dalia::BiquadParams m_params;
		// IDK what standard values are for effects like these...
		float m_frequency = 800.0f;
		float m_resonance = 0.7f;

	};
}