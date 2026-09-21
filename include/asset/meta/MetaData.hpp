#pragma once

#include "asset/GUID.hpp"

#include <json/json.hpp>
#include <cstdint>

using json = nlohmann::json;

namespace Droplet
{
    // TODO: Remove this temporary definition when edvin merges resource branch
    enum class ResourceType : uint8_t
    {
        None,
        Texture2D,
        Texture3D,
        TextureCube,
        TextureAtlas,
        Mesh,
        SkinnedMesh,
        Animation,
        Shader,
        Font,
        Material
    };

    /// @brief Determines if a resource is loaded into RAM, VRAM or both.
    enum class ResourceLoadFlag : uint8_t
    {
        LoadCPU = 1 << 0,
        LoadGPU = 1 << 0,
        LoadBoth  = LoadCPU
    };
    
    // Bitwise or
    inline constexpr ResourceLoadFlag operator|(ResourceLoadFlag a, ResourceLoadFlag b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    
    // Bitwise and
    inline constexpr ResourceLoadFlag operator&(ResourceLoadFlag a, ResourceLoadFlag b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
    
    // Bitwise xor
    inline constexpr ResourceLoadFlag operator^(ResourceLoadFlag a, ResourceLoadFlag b)
    {
        return static_cast<ResourceLoadFlag>(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
    }
    
    // Bitwise not
    inline constexpr ResourceLoadFlag operator~(ResourceLoadFlag a)
    {
        return static_cast<ResourceLoadFlag>(~static_cast<uint8_t>(a));
    }
    
    inline constexpr bool HasFlag(ResourceLoadFlag flags, ResourceLoadFlag flagToCheck)
    {
        return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flagToCheck)) != 0;
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
        json typeSpecificData = json::object();
    };
}