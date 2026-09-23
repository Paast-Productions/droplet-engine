#pragma once

#include "resource/GUID.hpp"
#include "resource/IResource.hpp"

#include <json/json.hpp>
#include <cstdint>

namespace Droplet
{
    /// @brief Determines if a resource is loaded into RAM, VRAM or both.
    enum class ResourceLoadFlag : uint8_t
    {
        LoadCPU = 1 << 0,
        LoadGPU = 1 << 0,
        LoadBoth  = LoadCPU
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

    /// @brief Represents a resource within a metafile.
    struct MetaEntry
    {
        // Common
        GUID guid = C_INVALID_GUID;
        ResourceType type = ResourceType::None;
        std::string name;
        ResourceLoadFlag loadFlags = ResourceLoadFlag::LoadCPU;
        std::vector<GUID> dependencies;
        
        // Resource specific
        nlohmann::json typeSpecificData = nlohmann::json::object();
    };
}