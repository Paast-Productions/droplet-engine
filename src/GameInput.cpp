#include "GameInput.hpp"

namespace Droplet
{
    void GameInput::Update()
    {
        m_previousKeys = m_currentKeys;

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
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
}
