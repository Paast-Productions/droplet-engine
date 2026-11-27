#pragma once
#include <Audio/Effect.hpp>
#include <json/json.hpp>
#include <dalia/Effect.hpp>


namespace Droplet::Audio
{
	/// @brief Maybe reverb maybe not i needed a test effect --Gupta the dumb one
	class ReverbEffect : public Effect
	{
	public:
		ReverbEffect() = default;
		~ReverbEffect() = default;

		[[nodiscard]] std::string GetEffectTypeName() const override { return "ReverbEffect"; }

	protected:
		void StartImpl();
		void RenderUIImpl();
		
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_json) override;
		
	private:
		dalia::FdnReverbParams m_params;
	};


}
