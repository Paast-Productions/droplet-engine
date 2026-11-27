/**
 * @file SDLWindow.hpp
 * @date 2026-10-20
 * @brief Declares an SDL state + window wrapper and a config struct for constructing the window.
 */
#pragma once

#include <functional>
#include <SDL3/SDL.h>

namespace Droplet::Platform::SDL
{
    /// Wrapper class for SDL window. Initializes SDL and creates a window.
    class Window
    {
    public:
        /// Configuration struct for setting up the window
        struct Config
        {
            // -- SDL --
            SDL_InitFlags initFlags{ SDL_INIT_VIDEO }; ///< At least SDL_INIT_VIDEO for rendering
            // -- Window --
            const char * title{ "Droplet Engine" }; ///< text in title bar
            int width{ 1280 };  ///< window width
            int height{ 720 };  ///< window height
            float dpiScale {1.0f}; ///< set on window creation. Automatically set if highDpi flag is set to true
            SDL_WindowFlags flags{ SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE }; ///< at leastSDL_WINDOW_VULKAN is required for Vulkan rendering
            bool highDpi {true}; ///< bool flags, these override creation flags
        };
    public:
        /// Takes a configuration struct and initializes SDL
        /// @throws std::runtime_error if SDL_Init failed
        Window(const Config& p_cfg);
        /// Destroys held window and calls SDL_Quit  
        ~Window();
        
        /// Get the internal SDL_Window, creating it on the first call
        SDL_Window* GetSDLWindow();

        // -- event callbacks --
        /// Callback for what happens when windows resizes
        void SetResizeCallback(const std::function<void()>& p_callback);
        /// External callback for when window minimizes
        void SetMinimizeCallback(const std::function<void()>& p_callback);
        /// External callback for what happens when window comes back from a minimize event
        void SetRestoreCallback(const std::function<void()>& p_callback);
        /// External callback for what happens when a key is pressed
        void SetKeyDownCallback(const std::function<void(SDL_Keycode)>& p_callback);
        /// External callback that sees every event first, before the callbacks above (the editor's ImGui)
        void SetEventCallback(const std::function<void(const SDL_Event&)>& p_callback);
        
        /// Poll for events and sets running to false if a quit message was caught or ESCAPE was pressed
        void PollEvents(bool* running) const;

        /// Blocks until at least one event is queued, without removing it. The next PollEvents handles it.
        /// @note Does not need an instance of window, can be called statically.
        static void WaitForEvent();
    private:
        /// Creates a single window instance when GetWindow is called.
        SDL_Window* CreateOwnedWindow();

    private:
        SDL_Window* m_window{ nullptr }; ///< stored window instance
        
        // callbacks
        std::function<void()> m_resizeCallback;
        std::function<void()> m_minimizeCallback;
        std::function<void()> m_restoreCallback;
        std::function<void(SDL_Keycode)> m_keyDownCallback;
        std::function<void(const SDL_Event&)> m_eventCallback;
        
        Config m_cfg; ///< stored config data
    };
}
