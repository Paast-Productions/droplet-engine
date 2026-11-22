/// @file AssetTransfer.hpp
/// @brief What a consumer requires, what it holds (resident) and the asset data handed to it (uploads); the
///        consumer never reads the asset store itself.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "Integration/Common/RenderDesc.hpp"
#include "Integration/Core/Guid.hpp"

namespace Assets
{
    /// @brief A mesh to upload. The spans point at CPU-owned data that only has to live until the upload call
    ///        returns: uploads are synchronous and the receiver keeps its own copy.
    struct MeshUpload
    {
        Droplet::GUID id{ Droplet::C_INVALID_GUID };
        std::uint64_t revision{ 0 }; ///< the storage's revision the data is from
        std::span<const std::byte> vertexData; ///< vertices back to back
        std::uint32_t vertexStride{ 0 };         ///< bytes per vertex
        std::span<const std::uint32_t> indices;
    };

    /// @brief A texture to upload; pixels as for MeshUpload's spans.
    struct TextureUpload
    {
        Droplet::GUID id{ Droplet::C_INVALID_GUID };
        std::uint64_t revision{ 0 }; ///< the storage's revision the data is from
        std::uint32_t width{0};
        std::uint32_t height{0};
        Common::PixelFormat format{Common::PixelFormat::RGBA8_SRGB};
        std::span<const std::byte> pixels; ///< tightly packed RGBA8, row 0 = top
        Common::TextureSampling sampling{};
    };

    /// @brief A scene shader to compile (a small description, copied).
    struct ShaderUpload
    {
        Droplet::GUID id{ Droplet::C_INVALID_GUID };
        std::uint64_t revision{ 0 }; ///< the storage's revision the asset is from
        Common::ShaderDesc shader;
    };

    /// @brief Asset data handed to the renderer for one frame.
    struct AssetUploads
    {
        std::vector<MeshUpload> meshes;
        std::vector<TextureUpload> textures;
        std::vector<ShaderUpload> shaders;
    };

    /// @brief The assets a world refers to, each once.
    struct RequiredAssets
    {
        std::vector<Droplet::GUID> meshes;
        std::vector<Droplet::GUID> textures;
        std::vector<Droplet::GUID> shaders;
    };

    /// @brief An asset on the GPU and the storage revision it was uploaded from.
    struct ResidentAsset
    {
        Droplet::GUID id{ Droplet::C_INVALID_GUID };
        std::uint64_t revision{ 0 };
    };

    /// @brief What the renderer holds, for the uploader to compare with the asset storage.
    struct ResidentAssets
    {
        std::vector<ResidentAsset> meshes;
        std::vector<ResidentAsset> textures;
        std::vector<ResidentAsset> shaders;       ///< compiled
        std::vector<ResidentAsset> failedShaders; ///< did not compile, at this revision
    };
}
