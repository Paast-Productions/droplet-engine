#pragma once
#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
	class TextComponent : public Component
	{
	public:
		TextComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "TextComponent"; }

	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
	};
}