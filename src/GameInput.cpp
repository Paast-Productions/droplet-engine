#include "GameInput.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>
#include <iostream>
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet
{
    static constexpr float C_EPSILON = 0.0001f;

    void GameInput::Update()
    {
        ZoneScoped;

        m_previousKeys = m_currentKeys;
        m_previousMouse = m_currentMouse;
        m_prevMouseX = m_mouseX;
        m_prevMouseY = m_mouseY;
        m_deltaMouseX = 0.0f;
        m_deltaMouseY = 0.0f;

		if (m_cursorLockWindow)
		{
			m_mouseX = m_lockedCursorX;
			m_mouseY = m_lockedCursorY;

			if (!m_firstLockFrame)
			{
                m_prevMouseX = m_lockedCursorX;
                m_prevMouseY = m_lockedCursorY;
			}

            SDL_WarpMouseInWindow(m_cursorLockWindow, static_cast<int>(m_lockedCursorX), static_cast<int>(m_lockedCursorY));
			m_firstLockFrame = false;
		}
    }

    void GameInput::ProcessEvent(const SDL_Event &p_event)
    {
        ZoneScoped;

        switch (p_event.type)
        {
		case SDL_EVENT_KEY_DOWN:
            // Ignore key repeat events.
            if (!p_event.key.repeat)
            {
                m_currentKeys.insert(static_cast<Key>(p_event.key.scancode));
            }
			break;

		case SDL_EVENT_KEY_UP:
            m_currentKeys.erase(static_cast<Key>(p_event.key.scancode));
			break;

		case SDL_EVENT_MOUSE_MOTION:
            m_mouseX = p_event.motion.x;
            m_mouseY = p_event.motion.y;
            m_deltaMouseX = p_event.motion.xrel;
			m_deltaMouseY = p_event.motion.yrel;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
            m_currentMouse[static_cast<std::size_t>(p_event.button.button)] = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
            m_currentMouse[static_cast<std::size_t>(p_event.button.button)] = false;
			break;

        default:
            break;
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
        //return m_mouseX - m_prevMouseX;
		return m_deltaMouseX;
    }

    float GameInput::GetDeltaMouseY() const
    {
        //return m_mouseY - m_prevMouseY;
		return m_deltaMouseY;
    }

    glm::vec2 GameInput::GetDeltaMouse() const
    {
		return glm::vec2(GetDeltaMouseX(), GetDeltaMouseY());
    }

    void GameInput::HideCursor(bool p_active)
    {
        if (p_active)
        {
            SDL_HideCursor();

        }
        else
        {
            SDL_ShowCursor();
        }
    }

    void GameInput::LockCursor(bool p_lock, float p_x, float p_y, SDL_Window *p_window)
    {
		if (!p_lock)
		{
			SDL_SetWindowRelativeMouseMode(m_cursorLockWindow, false);

			m_cursorLockWindow = nullptr;
            m_firstLockFrame = false;
			return;
		}

		m_firstLockFrame = true;
		m_cursorLockWindow = p_window ? p_window : m_mainWindow;

		m_lockedCursorX = p_x;
		m_lockedCursorY = p_y;

        SDL_SetWindowRelativeMouseMode(m_cursorLockWindow, true);
		SDL_WarpMouseInWindow(m_cursorLockWindow, static_cast<int>(m_lockedCursorX), static_cast<int>(m_lockedCursorY));
    }

    void GameInput::SetCursorPosition(SDL_Window *&p_window, bool p_relativeMouse, float p_x, float p_y)
    {
        float localX{}, localY{};

        if (p_relativeMouse)
        {
            // calculate mousepos in SDL window
            int windowX, windowY;
            SDL_GetWindowPosition(p_window, &windowX, &windowY);
            localX = p_x - static_cast<float>(windowX);
            localY = p_y - static_cast<float>(windowY);
        }
 
        // Set mousepos in global space (faster than local)
        SDL_WarpMouseGlobal(p_x, p_y);

        if (p_relativeMouse)
        {
            // reset the mouse to center position in SDL window
            m_mouseX = localX;
            m_mouseY = localY;
        }
    }
}
