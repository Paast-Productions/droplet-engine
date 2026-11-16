/**
 * @file SDLState.hpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Contains an SDL Wrapper for the sandbox environment.
 */
#pragma once

#include <functional>
#include <SDL3/SDL.h>

#include <string>

namespace SDL
{
    /// Wrapper class for SDL window. Initializes SDL and creates a window.
    class SDLWindow
    {
    public:
        /// Configuration struct for setting up the window
        struct Config
        {
            SDL_InitFlags flags{ SDL_INIT_VIDEO };
            
            /// window creation data
            struct SDLWindow
            {
                const char * name{ "Sandbox" };
                int width{ 1280 };
                int height{ 720 }; 
                float dpiScale {1.0f};
                SDL_WindowFlags flags{ SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE }; // SDL_WINDOW_VULKAN is required for Vulkan rendering
                bool highDpi {true}; ///< bool flags, these override creation flags
            }window{};
        };
    public:
        SDLWindow(Config cfg);
        ~SDLWindow();
        
        /// Get the internal SDL_Window
        SDL_Window* GetSDLWindow();

        // -- event callbacks --
        /// Callback for what happens when windows resizes
        void SetResizeCallback(const std::function<void()>& callback);
        /// External callback for when window minimizes
        void SetMinimizeCallback(const std::function<void()>& callback);
        /// External callback for what happens when window comes back from a minimize
        void SetRestoreCallback(const std::function<void()>& callback);
        /// External callback for what happens when a key is pressed
        void SetKeyDownCallback(const std::function<void(SDL_Keycode)>& callback);
        
        /// Poll for events and sets running to false if a quit message was caught or ESCAPE was pressed
        void PollEvents(bool* running) const;

        /// Blocks until at least one event is queued, without removing it. The next PollEvents handles it
        void WaitForEvent() const;
    private:
        /// Creates a single window instance when GetWindow is called.
        SDL_Window* CreateOwnedWindow();

    private:
        SDL_Window* m_window{ nullptr };
        
        // callbacks
        std::function<void()> m_resizeCallback;
        std::function<void()> m_minimizeCallback;
        std::function<void()> m_restoreCallback;
        std::function<void(SDL_Keycode)> m_keyDownCallback;
        
        Config m_cfg;
    };
}
