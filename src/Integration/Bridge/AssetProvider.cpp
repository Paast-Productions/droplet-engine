/**
 * @file AssetProvider.cpp
 * @brief Bridge::RequiredAssetsOf implementation
 */
#include "Integration/Bridge/AssetProvider.hpp"

#include <algorithm>
#include <vector>

namespace Bridge
{
    namespace
    {
        /// @brief Adds a valid `id` to `ids` unless it is already there
        void AddUniqueId(std::vector<Droplet::GUID>& ids, Droplet::GUID id)
        {
            if (id != Droplet::C_INVALID_GUID && std::ranges::find(ids, id) == ids.end())
                ids.push_back(id);
        }

        /// @brief Adds the texture `image` refers to; a render target is not an asset
        void AddRequiredTexture(Assets::RequiredAssets& required, const Common::ImageRef& image)
        {
            if (image.kind == Common::ImageRef::Kind::Texture)
                AddUniqueId(required.textures, image.id);
        }
    }

    Assets::RequiredAssets RequiredAssetsOf(const Render::RenderWorld& world, std::span<const Droplet::GUID> images)
    {
        Assets::RequiredAssets required;
        AddUniqueId(required.shaders, Render::DEFAULT_SCENE_SHADER);

        for (const Droplet::Graphics::CameraRenderData& camera : world.cameras)
        {
            for (const Droplet::Graphics::RenderData& object : camera.renderData)
        {
                AddUniqueId(required.meshes, object.meshData.GUID);
                AddRequiredTexture(required, Render::ImageOf(object.texData));
                AddUniqueId(required.shaders, Render::ShaderOf(object));
            }
        }
        for (const Droplet::GUID image : images)
        {
            const bool isTarget = std::ranges::any_of(world.cameras, [image](const Droplet::Graphics::CameraRenderData& camera)
            {
                return camera.renderTarget.id == image;
            });
            if (!isTarget)
                AddUniqueId(required.textures, image);
        }
        return required;
    }
}
