#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace Droplet::Editor::FileExplorer
{
    std::vector<std::filesystem::path> OpenFileDialog(bool p_allowMultiple, 
        const std::filesystem::path &p_defaultDir = std::filesystem::path());
}
#