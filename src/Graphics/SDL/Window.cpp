#include "Graphics/SDL/Window.hpp"

#include <SDL3/SDL_vulkan.h>

using namespace Droplet::Graphics::SDL;

Window::Window([[maybe_unused]] std::int32_t p_width, [[maybe_unused]] std::int32_t p_height, [[maybe_unused]] SDL_WindowFlags p_flags) 
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw SDLException("Failed");
    }
    if (!SDL_Vulkan_LoadLibrary(nullptr))
    {
        throw SDLException("Failed");
    }
    
    
    
}
