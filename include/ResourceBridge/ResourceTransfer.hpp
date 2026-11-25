/// @file ResourceTransfer.hpp
/// @brief What a consumer requires, what it holds (resident) and the asset data handed to it (uploads); the
///        consumer never reads the asset store itself.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "Common/RenderDesc.hpp"
#include "Core/Guid.hpp"

namespace Droplet::Graphics::Render
{
    struct RenderWorld;
}

/// @namespace Droplet::Bridge contains types to upload resources from CPU to GPU
namespace Droplet::Bridge
{
    /// @brief A mesh to upload. The spans point at CPU-owned data that only has to live until the upload call
    ///        returns: uploads are synchronous and the receiver keeps its own copy.
    struct MeshUpload
    {
        GUID id{C_INVALID_GUID};
        std::uint64_t revision{0}; ///< the rscMan's revision the data is from
        std::span<const std::byte> vertexData; ///< vertices back to back
        std::uint32_t vertexStride{0}; ///< bytes per vertex
        std::span<const std::uint32_t> indices;
    };

    /// @brief A texture to upload; pixels as for MeshUpload's spans.
    struct TextureUpload
    {
        GUID id{C_INVALID_GUID};
        std::uint64_t revision{0}; ///< the rscMan's revision the data is from
        std::uint32_t width{0};
        std::uint32_t height{0};
        Common::PixelFormat format{Common::PixelFormat::RGBA8_SRGB};
        std::span<const std::byte> pixels; ///< tightly packed RGBA8, row 0 = top
        Common::TextureSampling sampling{};
    };

    /// @brief A shader stage a shader asset holds
    enum class ShaderStage : std::uint8_t
    {
        Vertex,
        Fragment
    };

    /// @brief One compiled shader stage
    struct ShaderUpload
    {
        GUID id{ C_INVALID_GUID };
        std::uint64_t revision{ 0 };       ///< the storage's revision the asset is from
        ShaderStage stage{ ShaderStage::Vertex };
        std::span<const std::byte> spirv;  ///< SPIR-V; valid until Renderer::Upload returns
        std::string entryPoint = "main";
    };

    /// @brief Asset data handed to the renderer for one frame.
    struct ResourceUploads
    {
        std::vector<MeshUpload> meshes;
        std::vector<TextureUpload> textures;
        std::vector<ShaderUpload> shaders;
    };

    /// @brief The assets a world refers to, each once.
    struct RequiredResources
    {
        std::vector<GUID> meshes;
        std::vector<GUID> textures;
        std::vector<GUID> shaders;
    };

    /// @brief An asset on the GPU and the rscMan revision it was uploaded from.
    struct ResidentResource
    {
        GUID id{C_INVALID_GUID};
        std::uint64_t revision{0};
    };

    /// @brief What the renderer holds, for the uploader to compare with the rscMan resources.
    struct ResidentResources
    {
        std::vector<ResidentResource> meshes;
        std::vector<ResidentResource> textures;
        std::vector<ResidentResource> shaders; ///< compiled
        std::vector<ResidentResource> failedShaders; ///< did not compile, at this revision
    };

    /**
    * @brief Assets required to draw the render world
    * @param world The render world being drawn
    * @param images ImGui images that are not included in the world
    * @returns A RequiredAssets struct with vectors of GUIDs for each asset type
    */
    [[nodiscard]] RequiredResources RequiredResourcesOf(
        const Graphics::Render::RenderWorld& world, std::span<const GUID> images = {});
}
