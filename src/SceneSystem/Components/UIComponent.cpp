#include <UIComponent.hpp>
#include "ImGui/imgui.h"
#include "GameInput.hpp"

#include "RenderData.hpp"

namespace Droplet::Scene
{
    void UIComponent::UpdateImpl()
    {
        
    }

    void UIComponent::RenderUIImpl()
    {
#ifdef _EDITOR
        //GameInput::Get().ProcessEvent(m_renderer.Event);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("Hover", &m_hover);
        ImVec2 cursor;
        if(m_hover)
        {
            cursor = { GameInput::Get().GetCursorX(), GameInput::Get().GetCursorY() };
        }
        ImGui::InputFloat2("Cursor pos", &cursor.x, "%.1f");
        if ((cursor.x >= 100 && cursor.x <= 1000) &&
            (cursor.y >= 100 && cursor.y <= 1000))
        {
            std::cout << "i am inside" << std::endl;
        }

        ImGui::TableNextColumn(); ImGui::Checkbox("Click", &m_pressed);
        if (m_pressed)
        {
            if (GameInput::Get().MousePressed(Mouse::LMB))
            {
                std::cout << "click" << std::endl;
            }
        }
        
        ImGui::TableNextColumn(); ImGui::Checkbox("held", &m_held);
        if (m_held)
        {
            if (GameInput::Get().MouseHeld(Mouse::LMB))
            {
                std::cout << "click" << std::endl;
            }
        }
#endif
    }

    nlohmann::json UIComponent::SerializeImpl()
    {
        nlohmann::json json;
        json["hover"]   = m_hover;
        json["pressed"] = m_pressed;
        json["held"]    = m_held;
        return json;
    }

    void UIComponent::DeserializeImpl(nlohmann::json p_compJson)
    {
        if (p_compJson.contains("hover")) m_hover = p_compJson["hover"];
        if (p_compJson.contains("pressed")) m_pressed = p_compJson["pressed"];
        if (p_compJson.contains("held")) m_held = p_compJson["held"];
    }
}