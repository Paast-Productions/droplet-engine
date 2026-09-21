#pragma once
#include <memory>
#include <stdexcept>
#include <print>

#include <SDL3/SDL.h>

namespace Droplet::Graphics::SDL
{
    /// @struct WindowConfig
    /// @brief SDL-Window Configuration\n{ .Width, .Height, .Flags }
    struct WindowConfig
    {
        std::int32_t Width;
        std::int32_t Height;
        std::uint64_t Flags;
    };
    
    /// @class Window
    /// @brief SDL-Window
    class Window
    {
    public:
        Window() = delete;
        
        /// @brief SDL-window constructor 
        /// @param p_windowConfig SDL-Window Configuration Struct\n{ .Width, .Height, .Flags }
        Window(WindowConfig p_windowConfig);
        
        Window(const Window& p_sdlWindow) = delete;
        Window& operator=(const Window& p_sdlWindow) = delete;
        Window(Window&& p_sdlWindow) = delete;
        Window& operator=(Window&& p_sdlWindow) = delete;
        
        ~Window();
        
        /// @returns Non-owning SDL_Window pointer
        [[nodiscard]]
        SDL_Window* GetWindowHandle();
    
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

