#pragma once
#include <SceneSystem/Component.hpp>
namespace Droplet::Scene
{
	class PointLightComponent : public Component
	{
	public:
		PointLightComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "PointLightComponent"; }

	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	};
}