#pragma once
#include <SceneSystem/Component.hpp>
#include "GameInput.hpp"

namespace Droplet::Scene
{
	class UIComponent : public Component
	{
	public:
		UIComponent() = default;
		~UIComponent() = default;

		[[nodiscard]] std::string_view GetTypeName() const override { return "UIComponent"; }

		//DOTO more relevent functions 
	protected:
		void UpdateImpl() override;

		void RenderUIImpl() override;
		nlohmann::json SerializeImpl() override;

	private:
		//DOTO if variabels is needed.
	};
}