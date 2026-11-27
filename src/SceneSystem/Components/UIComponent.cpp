#include <UIComponent.hpp>
#include "ImGui/imgui.h"
#include "GameInput.hpp"
#include "Engine.hpp"

#include "RenderData.hpp"

namespace Droplet::Scene
{
    bool UIComponent::IsPointInsideTriangle(glm::vec2 p_corner_a, glm::vec2 p_corner_b, glm::vec2 p_corner_c)
    {
        const glm::vec2 normCurser = GetCurserNormalize();

        float denominator =
            (p_corner_b.y - p_corner_c.y) * (p_corner_a.x - p_corner_c.x) +
            (p_corner_c.x -p_corner_b.x) * (p_corner_a.y - p_corner_c.y);

        if (denominator == 0.0f)
            return false;

        float alpha =
            ((p_corner_b.y - p_corner_c.y) * (normCurser.x - p_corner_c.x) +
                (p_corner_c.x - p_corner_b.x) * (normCurser.y - p_corner_c.y)) / denominator;

        float beta =
            ((p_corner_c.y - p_corner_a.y) * (normCurser.x - p_corner_c.x) +
                (p_corner_a.x - p_corner_c.x) * (normCurser.y - p_corner_c.y)) / denominator;

        float gamma = 1.0f - alpha - beta;

        bool result = alpha >= 0.0f &&
            beta >= 0.0f &&
            gamma >= 0.0f;

        return result;
    }

    bool UIComponent::IsPointInsideRectangle(glm::vec2 p_position, glm::vec2 p_size)
    {
        const glm::vec2 normCurser = GetCurserNormalize();

        bool result = (normCurser.x >= p_position.x &&
            normCurser.x <= p_position.x + p_size.x) &&
            (normCurser.y >= p_position.y &&
                normCurser.y <= p_position.y + p_size.y);
    	
        std::cout << "cursor: (" << normCurser.x << ", " << normCurser.y << ")"
            << " | rect pos: (" << p_position.x << ", " << p_position.y << ")"
            << " | rect size: (" << p_size.x << ", " << p_size.y << ")"
            << " | inside: " << std::boolalpha << result << '\n';

    	return result;
    }

    //This one good be wrong/
    glm::vec2 UIComponent::GetCurserNormalize()
    {
        //vi kan kolla om är i editorn eller inte
        if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
        {
            //TODO
           //få storlek och pos funnktionen heter renderimpl som finns i sceneviewwindow
            //Engine::GetGameViewRect();
        }
        else
        {
            int width = 0;
            int height = 0;
            if (SDL_Window* window = GameInput::Get().GetMainWindow())
            {
                SDL_GetWindowSizeInPixels(window, &width, &height);
            }
            return curser = { GameInput::Get().GetCursorX() / width, GameInput::Get().GetCursorY() / height };
        }
    }


    bool UIComponent::IsPointInsideCircle(glm::vec2 p_position, float p_radius)
    {
        const glm::vec2 normCurser = GetCurserNormalize();

        float dx = normCurser.x - p_position.x;
        float dy = normCurser.y - p_position.y;

        bool result = (dx * dx + dy * dy) <= (p_radius * p_radius);
        return result;
    }

    void UIComponent::UpdateImpl()
    {
        //I am not using it
    }

    void UIComponent::RenderUIImpl()
    {
#ifdef _EDITOR
        ImGui::TableNextColumn(); ImGui::Checkbox("Click", &m_pressed);
        if (m_pressed)
        {
            if (GameInput::Get().MousePressed(Mouse::LMB))
            {
                //TODO
                //Need do to something. But currently i do not know
            }
        }
#endif
    }

    nlohmann::json UIComponent::SerializeImpl()
    {
        nlohmann::json json;
        json["pressed"] = m_pressed;
        return json;
    }

    void UIComponent::DeserializeImpl(nlohmann::json p_compJson)
    {

    }
}