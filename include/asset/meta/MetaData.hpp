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
    
    enum class AssetLoadFlag : uint8_t
    {
        LoadCPU = 1 << 0,
        LoadGPU = 1 << 0,
        LoadBoth  = LoadCPU
    };
    
    // Bitwise or
    inline constexpr AssetLoadFlag operator|(AssetLoadFlag a, AssetLoadFlag b)
    {
        return static_cast<AssetLoadFlag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    
    // Bitwise and
    inline constexpr AssetLoadFlag operator&(AssetLoadFlag a, AssetLoadFlag b)
    {
        return static_cast<AssetLoadFlag>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
    
    inline constexpr AssetLoadFlag operator^(AssetLoadFlag a, AssetLoadFlag b)
    {
        return static_cast<AssetLoadFlag>(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
    }
    
    inline constexpr AssetLoadFlag operator~(AssetLoadFlag a)
    {
        return static_cast<AssetLoadFlag>(~static_cast<uint8_t>(a));
    }
    
    inline constexpr bool HasFlag(AssetLoadFlag flags, AssetLoadFlag flagToCheck)
    {
        return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flagToCheck)) != 0;
    }
    
    struct ResourceMetaData
    {
        // Common
        GUID guid = C_INVALID_GUID;
        ResourceType type;
        std::string name;
        AssetLoadFlag loadFlags = AssetLoadFlag::LoadBoth;
        std::vector<GUID> dependencies;
        
        // Resource specific
        json typeSpecificData; // Ignored by catalog, parsed by IAssetLoader
    };
}