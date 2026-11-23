/**
 * @file AssetProvider.hpp
 * @brief Bridge::RequiredAssetsOf: the assets a world needs on the GPU
 */
#pragma once
#include <span>

#include "Integration/Assets/AssetTransfer.hpp"
#include "Integration/Render/RenderWorld.hpp"

namespace Bridge
{
    /**
     * @brief Assets required to draw the render world
     * @param world The render world being drawn
     * @param images ImGui images that are not included in the world
     * @returns A RequiredAssets struct with vectors of GUIDs for each asset type
     */
    [[nodiscard]] Assets::RequiredAssets RequiredAssetsOf(
        const Render::RenderWorld& world, std::span<const Droplet::GUID> images = {});
}
