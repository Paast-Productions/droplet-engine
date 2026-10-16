#pragma once

#include "resource/GuidUtils.hpp"
#include "resource/IResource.hpp"
#include "resource/types/ShaderResource.hpp"

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
    
    namespace MetaLoadSettings
    {
        /// @brief A load setting for a resource.
        /// @tparam T The datatype of the setting.
        template<typename T>
        struct Setting
        {
            const char* key;
            const char* displayName;
            T defaultValue;
        };
        
        // --- Texture Settings ---
        inline constexpr Setting<bool> C_GENERATE_MIPMAPS = { "generate_mipmaps", "Generate Mipmaps", false };
        
        // --- Mesh Settings ---
        inline constexpr Setting<bool> C_GENERATE_NORMALS = { "generate_normals", "Generate Normals", true };
        inline constexpr Setting<bool> C_JOIN_IDENTICAL_VERTICES = { "join_identical_vertices", "Join Identical Vertices", true };
        inline constexpr Setting<bool> C_TRIANGULATE = { "triangulate", "Triangulate", true };
        
        // --- Animation Settings ---
        inline constexpr Setting<const char*> C_TARGET_ANIMATION = { "target", "Target", "" };
        
        // --- Shader Settings ---
        inline constexpr Setting<ShaderResource::ShaderType> C_SHADER_TYPE = { "shader_type", "Shader Type", ShaderResource::ShaderType::Vertex };
        
        /// @brief Defines the datatypes available in a resource's load settings.
        enum class Type
        {
            Bool,
            Int,
            Float,
            String,
            Enum
        };

        /// @brief Describes a single JSON property inside the load settings of a meta entry.
        struct Descriptor
        {
            std::string key;                        // JSON key
            std::string displayName;                // Editor formatted name
            Type type;                              // The datatype
        
            float minVal = 0.0f;                    // For sliders
            float maxVal = 0.0f;                    // For sliders
            std::vector<std::string> enumOptions;   // For dropdowns
        };
    }
}