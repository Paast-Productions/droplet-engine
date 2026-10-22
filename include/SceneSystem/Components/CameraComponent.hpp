#pragma once
#include <SceneSystem/Component.hpp>
namespace Droplet::Scene
{
	class CameraComponent : public Component
	{
	public:
		CameraComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "CameraComponent"; }

	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	};
}