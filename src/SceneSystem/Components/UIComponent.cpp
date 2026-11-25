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
        return {};
    }
}
//ui
//
//rendera en bild av sepbastain
//
//om det är hover knapp
//
//om det är knapp eller inte
//
//konponent