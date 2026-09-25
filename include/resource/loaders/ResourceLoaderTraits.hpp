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
    struct ResourceLoaderTraits
    {
        static_assert(sizeof(T) == 0, "No AssetLoaderTrait has been defined for this resource type.");
    };
    
    template<>
    struct ResourceLoaderTraits<Texture2DResource>
    {
        static std::unique_ptr<Texture2DResource> LoadCPU(const std::filesystem::path& p_assetPath)
        {
            return GliLoader::LoadTexture2D(p_assetPath);
        }
    };
    
    // TODO: Implement 3D texture loading
    // template<>
    // struct ResourceLoaderTraits<Texture2DResource>
    // {
    //     static std::unique_ptr<Texture3DResource> LoadCPU(const std::filesystem::path& p_assetPath)
    //     {
    //         return GliLoader::LoadTexture3D(p_assetPath);
    //     }
    // };
    
    // TODO: Implement material loading
    // template<>
    // struct ResourceLoaderTraits<MaterialResource>
    // {
    //     static std::unique_ptr<MaterialResource> LoadCPU(const std::filesystem::path& p_assetPath)
    //     {
    //         
    //     }
    // };
    
    
}