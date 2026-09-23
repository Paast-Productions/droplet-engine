#pragma once
#include <memory>
#include <stdexcept>
#include <print>

#include <SDL3/SDL.h>

namespace Droplet::Graphics::SDL
{
    /// @struct WindowConfig
    /// @brief SDL-Window Configuration\n
    /// Options:
    /// - std::int32_t      Width [0]
    /// - std::int32_t      Height [0]
    /// - SDL_WindowFlags   Flags [SDL_WINDOW_VULKAN]
    struct WindowConfig
    {
        std::int32_t Width {0};
        std::int32_t Height {0};
        SDL_WindowFlags Flags {SDL_WINDOW_VULKAN};
    };
    
    /// @class Window
    /// @brief SDL-Window
    class Window
    {
    public:
        Window() = delete;
        
        /// @brief SDL-window constructor 
        /// @param p_windowConfig SDL-Window Configuration Struct
        Window(WindowConfig p_windowConfig);
        
        Window(const Window& p_sdlWindow) = delete;
        Window& operator=(const Window& p_sdlWindow) = delete;
        Window(Window&& p_sdlWindow) = delete;
        Window& operator=(Window&& p_sdlWindow) = delete;
        
        ~Window();
        
        /// @brief Getter-function for a raw SDL_Window pointer
        /// @returns Non-owning SDL_Window pointer
        [[nodiscard]] SDL_Window* Get();
    
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

