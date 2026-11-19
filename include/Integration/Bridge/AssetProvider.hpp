/**
 * @file AssetProvider.hpp
 * @brief Bridge::RequiredAssetsOf (what a world needs) and Bridge::ProvideAssets (the data for it, from the asset storage).
 */
#pragma once
#include <span>

#include "Integration/Assets/Assets.hpp"
#include "Integration/Assets/AssetTransfer.hpp"
#include "Integration/Render/RenderWorld.hpp"

namespace Bridge
{
    /// @brief Every mesh, texture and shader `world` refers to, and the default scene
    ///        shader, which the asset storage may override. Render targets are not assets.
    /// @param images also shown outside the world (Renderer::ImGuiImages): the ones that are not the world's targets
    ///        are required as textures
    [[nodiscard]] Assets::RequiredAssets RequiredAssetsOf(
        const Render::RenderWorld& world, std::span<const Guid> images = {});

    /**
     * @brief The uploads for what `required` needs and `resident` lacks or holds an older revision of.
     * @param assets   where the data comes from; the spans in the result point into it, so it must outlive them
     * @param required what the world refers to (RequiredAssetsOf)
     * @param resident what the GPU holds (Renderer::Resident)
     * @details Independent of rendering: call it between frames, later from a thread pool. Meshes and textures are
     *          uploaded when the GPU lacks them or the storage has a newer revision. Shaders are uploaded when the GPU
     *          has not seen them, or they did not compile and the storage has a newer revision; compiled ones are not
     *          reloaded (no hot reload yet). An asset the storage does not have is left out: the renderer draws its
     *          placeholder.
     */
    [[nodiscard]] Assets::AssetUploads ProvideAssets(
        const Assets::AssetStorage& assets,
        const Assets::RequiredAssets& required,
        const Assets::ResidentAssets& resident);
}
