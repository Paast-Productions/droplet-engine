#pragma once

#include <SDL3/SDL_events.h>

namespace Droplet::Graphics::SDL
{
    class Event
    {
    public:
        Event() = default;
        Event(const Event &p_other) = delete;
        Event &operator=(const Event &p_other) = delete;
        Event(Event &&p_other) noexcept = delete;
        Event &operator=(Event &&p_other) noexcept = delete;

        ~Event() = default;

        /// @brief Getter-function for SDL event pointer
        /// @returna SDL Event pointer
        SDL_Event *Get() { return &m_event; }
    
    private:
        SDL_Event m_event {};
    
    };
}
