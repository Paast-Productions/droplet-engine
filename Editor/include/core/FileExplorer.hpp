#pragma once

#include <filesystem>
#include <vector>
#include <functional>
#include <mutex>
#include <SDL3/SDL.h>

namespace Droplet::Editor
{
    class FileExplorer
    {
    public:
        /// @brief Callback signature used in file dialogs.
        using FileSelectedCallback = std::function<void(const std::vector<std::filesystem::path>&)>;
        
        FileExplorer() = default;
        ~FileExplorer() = default;
        
        /// @brief Opens an asynchronous file explorer window using SDL3 and retrieves the selected path(s).
        /// @param p_window The active SDL window.
        /// @param p_allowMultiple Allow user to select multiple files.
        /// @param p_onFileSelected Callback that handles how the selected (absolute) paths should be handled.
        void OpenFileDialog(SDL_Window* p_window, bool p_allowMultiple, FileSelectedCallback p_onFileSelected);
        
        /// @brief Opens an asynchronous file explorer window using SDL3 and retrieves the selected path(s).
        /// @param p_window The active SDL window.
        /// @param p_allowMultiple Allow user to select multiple files.
        /// @param p_onFileSelected Callback that handles how the selected (absolute) paths should be handled.
        /// @param p_defaultDirectory Directory the file explorer should start in.
        void OpenFileDialog(SDL_Window* p_window, bool p_allowMultiple, 
            const std::filesystem::path &p_defaultDirectory,
            FileSelectedCallback p_onFileSelected);

        /// @brief Executes pending callbacks. Should be called once per frame in the editor main loop.
        void Update();
        
    private:
        std::mutex m_dialogMutex;
        std::vector<std::function<void()>> m_mainThreadCallbacks;

        /// @brief Context to pass both the class instance and a user callback through SDL's API.
        struct DialogContext
        {
            FileExplorer *instance;
            FileSelectedCallback userCallback;
        };

        // SDL callback
        static void SDLCALL SDL_DialogCallback(void* userdata, const char* const* filelist, int filter);
    };
    



}
#