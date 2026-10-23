/**
 * @file SandboxSDL.hpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Contains an SDL Wrapper for the sandbox environment.
 */
#pragma once

#include <functional>
#include <SDL3/SDL.h>

#include <string>
#include <vector>

namespace Droplet::Sandbox
{
    /// Wrapper class for SDL state. Initializes SDL and created a window.
    class SandboxSDL
    {
    public:
        /// Configuration struct for setting up SDL
        struct Config
        {
            SDL_InitFlags flags{ SDL_INIT_VIDEO };
            
            /// window creation data
            struct Window
            {
                const char * name{ "Sandbox" };
                int width{ 800 };
                int height{ 600 };
                float dpiScale {1.0f};
                SDL_WindowFlags flags{ SDL_WINDOW_VULKAN };
                // bool flags, override creation flags
                bool highDpi {true};
            }window{};
        };
    public:
        SandboxSDL(Config cfg);
        ~SandboxSDL();
        
        /// Return the stored window instance
        SDL_Window* GetWindow();
        void SetResizeCallback(const std::function<void()>& callback);
        
        /// Poll for events and sets running to false if a quit message was caught or ESCAPE was pressed.
        void PollEvents(bool* running);
    private:
        /// Creates a single window instance when GetWindow is called.
        SDL_Window* CreateOwnedWindow();

    private:
        SDL_Window* m_window{ nullptr };
        std::function<void()> m_resizeCallback;
        Config m_cfg;
    };
}
