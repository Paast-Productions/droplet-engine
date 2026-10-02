#pragma once

#include "resource/GuidUtils.hpp"
#include "resource/IResource.hpp"

#include <json/json.hpp>
#include <cstdint>

namespace Droplet
{
    /// @brief Determines if a resource is loaded into RAM, VRAM or both.
    enum class ResourceLoadFlag : std::uint8_t
    {
        LoadCPU  = 1 << 0,           // Binary: 01
        LoadGPU  = 1 << 1,           // Binary: 10
        LoadBoth = LoadCPU | LoadGPU // Binary: 11
    };
    
    // Bitwise or
    inline constexpr ResourceLoadFlag operator|(ResourceLoadFlag p_a, ResourceLoadFlag p_b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(p_a) | static_cast<uint8_t>(p_b));
    }
    
    // Bitwise and
    inline constexpr ResourceLoadFlag operator&(ResourceLoadFlag p_a, ResourceLoadFlag p_b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(p_a) & static_cast<uint8_t>(p_b));
    }
    
    // Bitwise xor
    inline constexpr ResourceLoadFlag operator^(ResourceLoadFlag p_a, ResourceLoadFlag p_b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(p_a) ^ static_cast<uint8_t>(p_b));
    }
    
    // Bitwise not
    inline constexpr ResourceLoadFlag operator~(ResourceLoadFlag p_a)
    {
        return static_cast<ResourceLoadFlag>(~static_cast<uint8_t>(p_a));
    }
    
    inline constexpr bool HasFlag(ResourceLoadFlag p_flags, ResourceLoadFlag p_flagToCheck)
    {
        return (static_cast<uint8_t>(p_flags) & static_cast<uint8_t>(p_flagToCheck)) != 0;
    }

    /// @brief Represents a resource.
    struct MetaEntry
    {
        GUID guid = C_INVALID_GUID; // The globally unique identifier of this resource
        ResourceType type = ResourceType::None; // The type of resource this entry represents
        std::string name; // The name of the resource (found in the asset file or generated based on the asset file name)
        std::string relAssetPath; // Not written to metafile
        ResourceLoadFlag loadFlags = ResourceLoadFlag::LoadCPU; // Determines how the resource is stored in memory 
        std::vector<GUID> dependencies; // Currently unused
        
        nlohmann::json loadSettings = nlohmann::json::object(); // The resource-specific load settings to be used when loading this resource
    };
}