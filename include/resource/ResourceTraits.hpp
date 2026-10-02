#pragma once

#include "resource/loaders/AssimpLoader.hpp"
#include "resource/loaders/GliLoader.hpp"
#include "resource/loaders/SlangLoader.hpp"

#include "resource/types/Texture2DResource.hpp"
#include "resource/types/Texture3DResource.hpp"
#include "resource/types/MaterialResource.hpp"
#include "resource/types/AnimationResource.hpp"
#include "resource/types/MeshResource.hpp"
#include "resource/types/SkinnedMeshResource.hpp"
#include "resource/types/ShaderResource.hpp"

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
        static std::unique_ptr<Texture2DResource> LoadCPU(
            const std::filesystem::path& p_assetPath, 
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return GliLoader::LoadTexture2D(p_assetPath, p_loadSettings);
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
        static std::unique_ptr<MeshResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return AssimpLoader::LoadMesh(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }
        }
    };
    
    /// @brief Resource traits for SkinnedMeshResource.
    template<>
    struct ResourceTraits<SkinnedMeshResource>
    {
        static std::unique_ptr<SkinnedMeshResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return AssimpLoader::LoadSkinnedMesh(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }

        }
    };
    
    /// @brief Resource traits for AnimationResource.
    template<>
    struct ResourceTraits<AnimationResource>
    {
        static std::unique_ptr<AnimationResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return AssimpLoader::LoadAnimation(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }

        }
    };
    
    // --- Shaders ---
    
    /// @brief Resource traits for ShaderResource.
    template<>
    struct ResourceTraits<ShaderResource>
    {
        static std::unique_ptr<ShaderResource> LoadCPU(
            const std::filesystem::path &p_assetPath,
            const nlohmann::json &p_loadSettings)
        {
            try
            {
                return SlangLoader::CompileAndLoad(p_assetPath, p_loadSettings);
            }
            catch (std::exception &)
            {
                // TODO: Log error
                return nullptr;
            }

        }
    };
}