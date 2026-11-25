/**
 * @file ResourceTransfer.cpp
 * @brief ResourceTransfer implementation
 */
#include "ResourceBridge/ResourceTransfer.hpp"
#include "Common/RenderDesc.hpp"
#include "Graphics/Render/RenderWorld.hpp"

#include <algorithm>
#include <vector>

namespace Droplet::Bridge
{
    namespace
    {
        /// @brief Adds a valid `id` to `ids` unless it is already there
        void AddUniqueId(std::vector<GUID>& ids, GUID id)
        {
            if (id != C_INVALID_GUID && std::ranges::find(ids, id) == ids.end())
                ids.push_back(id);
        }

        /// @brief Adds the texture `image` refers to; a render target is not an asset
        void AddRequiredTexture(RequiredResources& required, const Common::ImageRef& image)
        {
            if (image.kind == Common::ImageRef::Kind::Texture)
                AddUniqueId(required.textures, image.id);
        }
    }

    RequiredResources RequiredResourcesOf(const Graphics::Render::RenderWorld& world, std::span<const GUID> images)
    {
        RequiredResources required;
        AddUniqueId(required.shaders, Graphics::Render::DEFAULT_SCENE_SHADER);

        for (const Common::CameraRenderData& camera : world.cameras)
        {
            for (const Common::RenderData& object : camera.renderData)
        {
                AddUniqueId(required.meshes, object.meshData.GUID);
                AddRequiredTexture(required, Graphics::Render::ImageOf(object.texData));
                AddUniqueId(required.shaders, Graphics::Render::ShaderOf(object));
            }
        }
        for (const GUID image : images)
        {
            const bool isTarget = std::ranges::any_of(world.cameras, [image](const Common::CameraRenderData& camera)
            {
                return camera.target.id == image;
            });
            if (!isTarget)
                AddUniqueId(required.textures, image);
        }
        return required;
    }
}
