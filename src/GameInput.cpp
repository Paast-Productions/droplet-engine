#include "GameInput.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>
#include <iostream>

namespace Droplet
{
    static constexpr float C_EPSILON = 0.0001f;

    void GameInput::Update()
    {
        m_previousKeys = m_currentKeys;
        m_previousMouse[static_cast<std::size_t>(Mouse::LMB)] = m_currentMouse[static_cast<std::size_t>(Mouse::LMB)];
        m_previousMouse[static_cast<std::size_t>(Mouse::RMB)] = m_currentMouse[static_cast<std::size_t>(Mouse::RMB)];
        m_prevMouseX = m_mouseX;
        m_prevMouseY = m_mouseY;

    }

    void GameInput::ProcessEvent(const SDL_Event &p_event)
    {
        // Keyboard events
        if (p_event.type == SDL_EVENT_KEY_DOWN)
        {
            // Ignore key repeat events.
            if (!p_event.key.repeat)
            {
                m_currentKeys.insert(static_cast<Key>(p_event.key.scancode));
            }
        }
        else if (p_event.type == SDL_EVENT_KEY_UP)
        {
            m_currentKeys.erase(static_cast<Key>(p_event.key.scancode));
        }

        // Mouse events
        if (p_event.type == SDL_EVENT_MOUSE_MOTION)
        {
            m_mouseX = p_event.motion.x;
            m_mouseY = p_event.motion.y;
        }

        if (p_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (p_event.button.button == SDL_BUTTON_LEFT)
            {
                m_currentMouse[static_cast<std::size_t>(Mouse::LMB)] = true;
            }
            if (p_event.button.button == SDL_BUTTON_RIGHT)
            {
                m_currentMouse[static_cast<std::size_t>(Mouse::RMB)] = true;
            }
        }
        else if (p_event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            if (p_event.button.button == SDL_BUTTON_LEFT)
            {
                m_currentMouse[static_cast<std::size_t>(Mouse::LMB)] = false;
            }
            if (p_event.button.button == SDL_BUTTON_RIGHT)
            {
                m_currentMouse[static_cast<std::size_t>(Mouse::RMB)] = false;
            }
        }
    }

    bool GameInput::KeyPressed(const Key p_key)
    {
        return m_currentKeys.contains(p_key) && !m_previousKeys.contains(p_key);
    }

    bool GameInput::KeyHeld(const Key p_key)
    {
        return m_currentKeys.contains(p_key);
    }

    bool GameInput::KeyReleased(const Key p_key)
    {
        return !m_currentKeys.contains(p_key) && m_previousKeys.contains(p_key);
    }

    bool GameInput::KeyToggle(const Key p_key)
    {
        if (KeyPressed(p_key))
        {
            m_toggledKeys[p_key] = !m_toggledKeys[p_key];
        }

        return m_toggledKeys.contains(p_key) && m_toggledKeys[p_key];
    }

    bool GameInput::MousePressed(const Mouse p_mouse)
    {
        return m_currentMouse[static_cast<std::size_t>(p_mouse)] &&
            !m_previousMouse[static_cast<std::size_t>(p_mouse)];
    }

    bool GameInput::MouseHeld(const Mouse p_mouse)
    {
        return m_currentMouse[static_cast<std::size_t>(p_mouse)];
    }

    bool GameInput::MouseReleased(const Mouse p_mouse)
    {
        return !m_currentMouse[static_cast<std::size_t>(p_mouse)] &&
            m_previousMouse[static_cast<std::size_t>(p_mouse)];
    }

    float GameInput::GetCursorX() const
    {
        return m_mouseX;
    }

    float GameInput::GetCursorY() const
    {
        return m_mouseY;
    }

    float GameInput::GetDeltaMouseX() const
    {
        return m_mouseX - m_prevMouseX;
    }

    float GameInput::GetDeltaMouseY() const
    {
        return m_mouseY - m_prevMouseY;
    }

    void GameInput::SetCursorPosition(SDL_Window *&p_window, float p_x, float p_y)
    {
        m_mouseX = p_x;
        m_mouseY = p_y;
        SDL_WarpMouseInWindow(p_window, m_mouseX, m_mouseY);
    }
}
