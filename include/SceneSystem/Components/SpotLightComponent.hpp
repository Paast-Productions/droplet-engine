#pragma once
#include <SceneSystem/Component.hpp>
namespace Droplet::Scene
{
	class SpotLightComponent : public Component
	{
	public:
		SpotLightComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "SpotLightComponent"; }

	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	};
}