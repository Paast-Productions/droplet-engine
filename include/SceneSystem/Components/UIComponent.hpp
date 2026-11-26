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
		const std::string GetName() { return m_soundName; }

		bool IsPointInsideTriangle(glm::vec2 p_corner_a, glm::vec2 p_corner_b, glm::vec2 p_corner_c);
		bool IsPointInsideRectangle(glm::vec2 p_position, glm::vec2 p_size);
		bool IsPointInsideCircle(glm::vec2 p_position, float p_radius);
		
		//Get
		glm::vec2 GetCurserNormalize();

		//glm::vec2 GetOrthoCursorPos();


		//set
		void UIComponent::SetCurser(glm::vec2 p_curser) { curser = p_curser; };

		//DOTO more relevent functions 
	protected:
		void UpdateImpl() override;

		void RenderUIImpl() override;
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
		std::string m_soundName = "None";
		glm::vec2 curser={0,0};
		
		bool m_pressed = false;
	};
}