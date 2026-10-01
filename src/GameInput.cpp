#include "GameInput.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>

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

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            // Keyboard events
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                // Ignore key repeat events.
                if (!event.key.repeat)
                {
                    m_currentKeys.insert(static_cast<Key>(event.key.scancode));
                }
            }
            else if (event.type == SDL_EVENT_KEY_UP)
            {
                m_currentKeys.erase(static_cast<Key>(event.key.scancode));
            }

            // Mouse events
            if (std::abs(event.motion.x) >= C_EPSILON)
            {
                m_mouseX = event.motion.x;
            }
            if (std::abs(event.motion.y) >= C_EPSILON)
            {
                m_mouseY = event.motion.y;
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    m_currentMouse[static_cast<std::size_t>(Mouse::LMB)] = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    m_currentMouse[static_cast<std::size_t>(Mouse::RMB)] = true;
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    m_currentMouse[static_cast<std::size_t>(Mouse::LMB)] = false;
                }
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    m_currentMouse[static_cast<std::size_t>(Mouse::RMB)] = false;
                }
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
        return m_prevMouseX - m_mouseX;
    }

    float GameInput::GetDeltaMouseY() const
    {
        return m_prevMouseY - m_mouseY;
    }

    void GameInput::SetCursorPosition(SDL_Window *&p_window, float p_x, float p_y)
    {
        m_mouseX = p_x;
        m_mouseY = p_y;
        SDL_WarpMouseInWindow(p_window, m_mouseX, m_mouseY);
    }
}
