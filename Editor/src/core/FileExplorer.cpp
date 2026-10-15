#include "core/FileExplorer.hpp"

#include <PortableFileDialogs/portable-file-dialogs.h>

namespace Droplet::Editor
{
    std::vector<std::filesystem::path> FileExplorer::OpenFileDialog(bool p_allowMultiple,
        const std::filesystem::path &p_defaultDir)
    {
        std::string defaultDirStr = "";
        if (!p_defaultDir.empty())
        {
            std::filesystem::path nativePath = p_defaultDir;
            nativePath /= "";
            defaultDirStr = nativePath.make_preferred().string();
        }
        
        auto selection = pfd::open_file("Register Assets", defaultDirStr,
            { "All", "*",
                "Texture (.png, .jpg, .jpeg, .ktx, .dds)", "*.png *.jpg *.jpeg *.ktx *.dds",
            "Model (.obj, .fbx, .gltf)", "*.obj *.fbx *.gltf",
            "Shader (.slang)", ".slang"},
            p_allowMultiple ? pfd::opt::multiselect : pfd::opt::none
        ).result();
        
        std::vector<std::filesystem::path> paths;
        paths.reserve(selection.size());
        for (const auto &pathStr : selection)
        {
            paths.emplace_back(pathStr);
        }
        
        return paths;
    }
}
