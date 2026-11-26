/**
* @file SDLWindow.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Defines the Window wrapper
 */
#include "Platform/SDLWindow.hpp"

#include <algorithm>
#include <stdexcept>

using namespace Droplet::Platform::SDL;

Window::Window(const Config& p_cfg) : m_cfg{p_cfg}
{
    if (!SDL_Init(m_cfg.flags))
    {
        throw std::runtime_error("SDL failed to initialize.");
    }
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

SDL_Window* Window::GetSDLWindow()
{
    if (!m_window)
    {
        return CreateOwnedWindow();
    }
    return m_window;
}

void Window::SetResizeCallback(const std::function<void()>& callback)
{
    m_resizeCallback = callback;
}

void Window::SetMinimizeCallback(const std::function<void()>& callback)
{
    m_minimizeCallback = callback;
}

void Window::SetRestoreCallback(const std::function<void()>& callback)
{
    m_restoreCallback = callback;
}

void Window::SetKeyDownCallback(const std::function<void(SDL_Keycode)>& callback)
{
    m_keyDownCallback = callback;
}

void Window::SetEventCallback(const std::function<void(const SDL_Event&)>& callback)
{
    m_eventCallback = callback;
}

void Window::PollEvents(bool* running) const
{
    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        if (m_eventCallback)
        {
            m_eventCallback(event);
        }

        if (event.type == SDL_EVENT_QUIT)
        {
            *running = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            *running = false;
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            m_resizeCallback();
        }
        if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
        {
            m_minimizeCallback();
        }
        // un-minimizing a maximized window reports MAXIMIZED rather than RESTORED, so both end a minimized state
        if (event.type == SDL_EVENT_WINDOW_RESTORED || event.type == SDL_EVENT_WINDOW_MAXIMIZED)
        {
            m_restoreCallback();
        }
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            m_keyDownCallback(event.key.key);
        }
    }
}

void Window::WaitForEvent()
{
    // A null event leaves the event in the queue.
    SDL_WaitEvent(nullptr);
}

SDL_Window* Window::CreateOwnedWindow()
{
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
    }

    auto& cfg = m_cfg;

    // set up window to account for higher dpi screens
    if (cfg.highDpi)
    {
        SDL_DisplayID primary = SDL_GetPrimaryDisplay();
        if (primary == 0)
        {
            throw std::runtime_error("Failed to get primary display");
        }

        float scale = SDL_GetDisplayContentScale(primary);
        if (scale > 1.0f && scale > cfg.dpiScale)
        {
            cfg.dpiScale = scale;
        }

        cfg.width = static_cast<int>(static_cast<float>(cfg.width) * cfg.dpiScale);
        cfg.height = static_cast<int>(static_cast<float>(cfg.height) * cfg.dpiScale);

        cfg.flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    }

    m_window = SDL_CreateWindow(
        cfg.title,
        cfg.width,
        cfg.height,
        cfg.flags
    );

    if (!m_window)
    {
        throw std::runtime_error("Window creation failed");
    }

    return m_window;
}
