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
        Window() { m_sdlWindowHandle = nullptr; }

        Window(nullptr_t p_nullptr)
        {
            m_sdlWindowHandle = { p_nullptr };
        }
        
        /// @brief SDL-window constructor 
        /// @param p_windowConfig SDL-Window Configuration Struct
        Window(WindowConfig p_windowConfig);
        
        ~Window();

        Window(const Window &p_other) = delete;
        Window &operator=(const Window &p_other) = delete;

        Window(Window &&p_other) noexcept
        {
            std::swap(m_sdlWindowHandle, p_other.m_sdlWindowHandle);
        };

        Window &operator=(Window &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }

            std::swap(m_sdlWindowHandle, p_other.m_sdlWindowHandle);

            return *this;
        };

        bool operator==(const Window &p_other) const
        {
            return (m_sdlWindowHandle == p_other.m_sdlWindowHandle);
        }
        
        /// @brief Getter-function for a raw SDL_Window pointer
        /// @returns Non-owning SDL_Window pointer
        [[nodiscard]] SDL_Window* Get() const;

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

