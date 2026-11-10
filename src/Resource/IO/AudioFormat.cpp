#include "Resource/IO/AudioFormat.hpp"

namespace Droplet::IO::AudioFormat
{
    std::vector<std::pair<ResourceType, std::string>> ScanAssetResources(const std::filesystem::path &p_assetPath)
    {
        ZoneScoped;
        
        std::vector<std::pair<ResourceType, std::string>> resourceList{};
        
        if (!std::filesystem::is_regular_file(p_assetPath))
        {
            throw std::runtime_error("Filepath does not represent an audio file.");
        }
        
        std::string resourceName = p_assetPath.stem().string();
        resourceList.emplace_back(ResourceType::Sound, resourceName);
        
        return resourceList;
    }
}
