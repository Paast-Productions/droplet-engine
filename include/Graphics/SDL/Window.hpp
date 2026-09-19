#pragma once
#include <memory>
#include <stdexcept>
#include <print>

#include <SDL3/SDL.h>

namespace Droplet::SDL
{
    /// @brief 
    class Window
    {
    public:
        Window() = delete;
        
        /// @brief SDL window constructor 
        /// @param p_width Width of the SDL window
        /// @param p_height Height of the SDL window
        /// @param p_flags Extra SDL-related flags
        Window(std::int32_t p_width, std::int32_t p_height, SDL_WindowFlags p_flags);
        
        Window(const Window& p_sdlWindow) = delete;
        Window& operator=(const Window& p_sdlWindow) = delete;
        Window(Window&& p_sdlWindow) = delete;
        Window& operator=(Window&& p_sdlWindow) = delete;
        
        ~Window() = default;
    
    private:
        /// @brief Custom deleter function for SDL_Window*. This enables the usage of std::unique_ptr<>
        struct WindowDeleter
        {
            void operator()(SDL_Window *p_window) const { SDL_DestroyWindow(p_window); }
        };
        
        /// @brief SDL exception wrapper
        struct SDLException final : std::runtime_error 
        {
            /// @brief Exception wrapper that takes SDL_GetError() into account when getting thrown
            /// @param message Custom error message
            explicit SDLException(std::string_view message) : 
            std::runtime_error
            (
                std::format("{}: {}", message, SDL_GetError())
            ) {}
        };
        
        std::unique_ptr<SDL_Window, WindowDeleter> m_sdlWindowHandle {};
    };
}

