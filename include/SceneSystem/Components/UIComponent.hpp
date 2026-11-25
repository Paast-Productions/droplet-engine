#pragma once
#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
	class UIComponent : public Component
	{
	public:
		UIComponent() = default;
		~UIComponent() = default;

		[[nodiscard]] std::string_view GetTypeName() const override { return "UIComponent"; }

		void CreateDynamicMesh();

		//DOTO more relevent functions 
	protected:
		void UpdateImpl() override;

		void RenderUIImpl() override;
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
		bool m_hover;
		bool m_pressed;
		bool m_held;
	};
}