#include "Graphics/SDL/Window.hpp"

#include <SDL3/SDL_vulkan.h>

using namespace Droplet::Graphics::SDL;

Window::Window(WindowConfig p_windowConfig) 
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw SDLException("Failed (SDL_INIT)");
    }
    
    if (!SDL_Vulkan_LoadLibrary(nullptr))
    {
        throw SDLException("Failed (SDL_Vulkan_LoadLibrary)");
    }
    
    m_sdlWindowHandle = std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow("Hello", p_windowConfig.Width, p_windowConfig.Height, p_windowConfig.Flags | SDL_WINDOW_VULKAN));
    
    if (not m_sdlWindowHandle)
    {
        throw SDLException("Failed to create window)");
    }
    
    SDL_SetWindowResizable(m_sdlWindowHandle.get(), true);
}

Window::~Window()
{
    SDL_Quit();
}

SDL_Window *Window::Get()
{
    return m_sdlWindowHandle.get();
}
