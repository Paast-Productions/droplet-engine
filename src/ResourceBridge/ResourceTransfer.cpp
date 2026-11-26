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

        /// @brief Adds the texture an object shows; a render target is not an asset
        void AddRequiredTexture(Bridge::RequiredResources& required, const Common::TextureData& texture)
        {
            if (!texture.isRenderTarget)
                AddUniqueId(required.textures, texture.GUID);
        }
    }

    RequiredResources RequiredResourcesOf(const Graphics::Render::RenderWorld& world, std::span<const GUID> images)
    {
        Bridge::RequiredResources required;
        AddUniqueId(required.shaders, world.defaultVertexShader);
        AddUniqueId(required.shaders, world.defaultFragmentShader);

        for (const Common::CameraRenderData& camera : world.cameras)
        {
            for (const Common::RenderData& object : camera.renderData)
            {
                AddUniqueId(required.meshes, object.meshData.GUID);
                AddRequiredTexture(required, object.texData);
                AddUniqueId(required.shaders, object.vsData.guid);
                AddUniqueId(required.shaders, object.tcsData.guid);
                AddUniqueId(required.shaders, object.tesData.guid);
                AddUniqueId(required.shaders, object.gsData.guid);
                AddUniqueId(required.shaders, object.fsData.guid);
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
