#include <UIComponent.hpp>
#include "ImGui/imgui.h"

namespace Droplet::Scene
{
    void UIComponent::UpdateImpl()
    {
        // TODO
    }

    void UIComponent::RenderUIImpl()
    {
#ifdef _EDITOR
        //GameInput::Get().ProcessEvent(m_renderer.Event);
        
        float cursor[2] = {
            GameInput::Get().GetCursorX(),
            GameInput::Get().GetCursorY()
        };

        ImGui::InputFloat2("Cursor pos", cursor, "%.1f");
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