#include "core/FileExplorer.hpp"

#include <SDL3/SDL.h>

namespace Droplet::Editor
{
    void FileExplorer::OpenFileDialog(SDL_Window *p_window, bool p_allowMultiple, FileSelectedCallback p_onFileSelected)
    {
        OpenFileDialog(p_window, p_allowMultiple, std::filesystem::path(), std::move(p_onFileSelected));
    }

    void FileExplorer::OpenFileDialog(SDL_Window *p_window, bool p_allowMultiple, 
                                      const std::filesystem::path &p_defaultDirectory, FileSelectedCallback p_onFileSelected)
    {
        // Heap allocation is necessary so that it survives async process
        auto* dialogCtx = new DialogContext{ this, std::move(p_onFileSelected) };
        
        SDL_DialogFileFilter fileFilters[] = {
            { "All Files", "*"},
            { "Texture Files (png, jpg, jpeg, ktx, dds)", "png;jpg;jpeg;ktx;dds"},
            { "Model Files (fbx, gltf, obj)", "fbx;gltf;obj"},
            { "Audio Files (.ogg)", "ogg"},
            { "Shader Files (slang)", "slang"}
        };
        
        std::string defaultDirStr;
        const char* defaultDirCStr = nullptr;
        if (!p_defaultDirectory.empty())
        {
            std::filesystem::path nativePath = p_defaultDirectory;
            nativePath /= "";
            defaultDirStr = nativePath.make_preferred().string();
            defaultDirCStr = defaultDirStr.c_str();
        }
        
        SDL_ShowOpenFileDialog(
            SDL_DialogCallback, // Internal callback
            dialogCtx,        
            p_window,
            fileFilters,
            5,
            defaultDirCStr,
            p_allowMultiple 
        );  
    }

    void FileExplorer::Update()
    {
        std::vector<std::function<void()>> callbacks;
        
        {
            std::scoped_lock lock(m_dialogMutex);
            if (m_mainThreadCallbacks.empty()) return;
            callbacks = std::move(m_mainThreadCallbacks);
        }
        
        for (const auto &cb : callbacks)
        {
            cb();
        }
    }

    inline void SDLCALL FileExplorer::SDL_DialogCallback(void* userdata, const char* const* filelist, int filter)
    {
        auto* dialogCtx = static_cast<DialogContext*>(userdata);
        
        std::vector<std::filesystem::path> paths;
        if (filelist)
        {
            for (int i = 0; filelist[i] != nullptr; i++)
            {
                paths.emplace_back(filelist[i]);
            }
        }
        
        // Push task into the file explorer instance queue
        {
            std::scoped_lock lock(dialogCtx->instance->m_dialogMutex);
            dialogCtx->instance->m_mainThreadCallbacks.push_back([
                callback = std::move(dialogCtx->userCallback), pathList = std::move(paths)]()
            {
                callback(pathList);
            });
        }
        
        delete dialogCtx; // Callback is heap-allocated so it must be deleted here
    }
}
