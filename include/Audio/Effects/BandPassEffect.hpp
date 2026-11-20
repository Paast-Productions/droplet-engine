#pragma once
#include <Audio/Effect.hpp>

namespace Droplet::Audio
{
	class BandPassEffect : public Effect
	{
	public:
		BandPassEffect() = default;
		~BandPassEffect() = default;

		[[nodiscard]] std::string GetEffectTypeName() const override { return "BandPassEffect"; }

	protected:

		void StartImpl() override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_josn) override;

	private:
		dalia::BiquadParams m_params;
		float m_frequency = 800.0f;
		float m_resonance = 0.7f;
	};
}