#pragma once

#include "resource/io/AssimpFormat.hpp"
#include "resource/io/GliFormat.hpp"
#include "resource/io/SlangFormat.hpp"

#include "resource/types/Texture2DResource.hpp"
#include "resource/types/Texture3DResource.hpp"
#include "resource/types/MaterialResource.hpp"
#include "resource/types/AnimationResource.hpp"
#include "resource/types/MeshResource.hpp"
#include "resource/types/SkinnedMeshResource.hpp"
#include "resource/types/ShaderResource.hpp"

#include <json/json.hpp>

namespace Droplet
{
    /// @brief Fallback template. If the compiler hits this, load was called on a type that doesn't have a defined
    /// load trait.
    /// @tparam T The resource.
    template<typename T>
    struct ResourceTraits
    {
        static_assert(sizeof(T) == 0, "No traits has been defined for this resource type.");
    };
    
    // --- Textures ---

    /// @brief Resource traits for Texture2DResource.
    template<>
    struct ResourceTraits<Texture2DResource>
    {
        static ResourceType GetType()
        {
            return ResourceType::Texture2D;
        }
        
        static std::unique_ptr<Texture2DResource> LoadCPU(
            const std::filesystem::path& p_assetPath, 
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return IO::GliFormat::LoadTexture2D(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
        
        static std::unique_ptr<Texture2DResource> CreateFallback()
        {
            return Texture2DResource::CreateFallback();
        }
    };
    
    
    // TODO: Implement 3D texture traits
    // template<>
    // struct ResourceLoaderTraits<Texture2DResource>
    // {
    //     static std::unique_ptr<Texture3DResource> LoadCPU(
    //          const std::filesystem::path& p_assetPath
    //          const nlohmann::json &p_loadSettings)
    //     {
    //         return GliLoader::LoadTexture3D(p_assetPath, p_loadSettings);
    //     }
    // };
    
    // TODO: Implement material traits
    // template<>
    // struct ResourceLoaderTraits<MaterialResource>
    // {
    //     static std::unique_ptr<MaterialResource> LoadCPU(
    //          const std::filesystem::path& p_assetPath
    //          const nlohmann::json &p_loadSettings)
    //     {
    //         
    //     }
    // };
    
    // --- Meshes and Animations ---
    
    /// @brief Resource traits for MeshResource.
    template<>
    struct ResourceTraits<MeshResource>
    {
        static ResourceType GetType()
        {
            return ResourceType::Mesh;
        }
        
        static std::unique_ptr<MeshResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return IO::AssimpFormat::LoadMesh(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
        
        static std::unique_ptr<MeshResource> CreateFallback()
        {
            return MeshResource::CreateFallback();
        }
    };
    
    /// @brief Resource traits for SkinnedMeshResource.
    template<>
    struct ResourceTraits<SkinnedMeshResource>
    {
        static ResourceType GetType()
        {
            return ResourceType::SkinnedMesh;
        }
        
        static std::unique_ptr<SkinnedMeshResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return IO::AssimpFormat::LoadSkinnedMesh(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
        
        static std::unique_ptr<SkinnedMeshResource> CreateFallback()
        {
            return SkinnedMeshResource::CreateFallback();
        }
    };
    
    /// @brief Resource traits for AnimationResource.
    template<>
    struct ResourceTraits<AnimationResource>
    {
        static ResourceType GetType()
        {
            return ResourceType::Animation;
        }
        
        static std::unique_ptr<AnimationResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return IO::AssimpFormat::LoadAnimation(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
        
        static std::unique_ptr<AnimationResource> CreateFallback()
        {
            return AnimationResource::CreateFallback();
        }
    };
    
    // --- Shaders ---
    
    /// @brief Resource traits for ShaderResource.
    template<>
    struct ResourceTraits<ShaderResource>
    {
        static ResourceType GetType()
        {
            return ResourceType::Shader;
        }
        
        static std::unique_ptr<ShaderResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return IO::SlangFormat::CompileAndLoad(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
        
        static std::unique_ptr<ShaderResource> CreateFallback()
        {
            return ShaderResource::CreateFallback();
        }
    };
}