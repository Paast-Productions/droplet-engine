/**
* @file SandboxSDL.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Source file for SandboxSDL.hpp.
 */
#include "SandboxSDL.hpp"

#include <algorithm>
#include <stdexcept>

using namespace Droplet::Sandbox;

SandboxSDL::SandboxSDL(Config cfg) : m_cfg{std::move(cfg)}
{
    if (!SDL_Init(m_cfg.flags))
        throw std::runtime_error("SDL failed to initialize.");
}

SandboxSDL::~SandboxSDL()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

SDL_Window* SandboxSDL::GetWindow()
{
    if (!m_window)
        return CreateOwnedWindow();

    return m_window;
}

void SandboxSDL::PollEvents(bool* running)
{
    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            *running = false;
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
            *running = false;
    }
}

SDL_Window* SandboxSDL::CreateOwnedWindow()
{
    if (m_window)
        SDL_DestroyWindow(m_window);

    auto& cfg = m_cfg.window;

    if (cfg.highDpi)
    {
        SDL_DisplayID primary = SDL_GetPrimaryDisplay();
        if (primary == 0)
            throw std::runtime_error("Failed to get primary display");

        float scale = SDL_GetDisplayContentScale(primary);
        if (scale > 1.0f && scale > cfg.dpiScale)
            cfg.dpiScale = scale;

        cfg.width = static_cast<int>(static_cast<float>(cfg.width) * cfg.dpiScale);
        cfg.height = static_cast<int>(static_cast<float>(cfg.height) * cfg.dpiScale);

        cfg.flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    }

    m_window = SDL_CreateWindow(
        cfg.name,
        cfg.width,
        cfg.height,
        cfg.flags
    );

    if (!m_window)
        throw std::runtime_error("Window creation failed");

    return m_window;
}
