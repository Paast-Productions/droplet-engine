/**
 * @file AssetProvider.cpp
 * @brief RequiredAssetsOf and ProvideAssets implementations.
 */
#include "Integration/Bridge/AssetProvider.hpp"

#include <algorithm>
#include <vector>

namespace Bridge
{
    namespace
    {
        /// @brief The entry for `id` in `assets`, or nullptr
        const Assets::ResidentAsset* Find(const std::vector<Assets::ResidentAsset>& assets, Guid id)
        {
            const auto assetIt = std::ranges::find(assets, id, &Assets::ResidentAsset::id);
            return assetIt != assets.end() ? &*assetIt : nullptr;
        }

        /// @brief Adds the mesh to uploads if it exists in the asset storage
        void AddMesh(const Assets::AssetStorage& assets, Assets::AssetUploads& uploads, Guid id)
        {
            const Assets::MeshAsset* mesh = assets.FindMesh(id);
            if (!mesh || mesh->vertexData.empty() || mesh->indices.empty())
                return; // not in the storage: the renderer draws its placeholder cube

            uploads.meshes.push_back({
                .id = id,
                .revision = assets.MeshRevision(id),
                .vertexData = mesh->vertexData,
                .vertexStride = mesh->vertexStride,
                .indices = mesh->indices });
        }

        /// @brief Adds the texture to uploads if it exists in the asset storage
        void AddTexture(const Assets::AssetStorage& assets, Assets::AssetUploads& uploads, Guid id)
        {
            const Assets::TextureAsset* texture = assets.FindTexture(id);
            if (!texture)
                return; // not in the storage: the renderer draws its placeholder texture

            uploads.textures.push_back({
                .id = id,
                .revision = assets.TextureRevision(id),
                .width = texture->width,
                .height = texture->height,
                .format = texture->format,
                .pixels = texture->pixels,
                .sampling = texture->sampling });
        }

        /// @brief Adds the shader to uploads if it exists in the asset storage
        void AddShader(const Assets::AssetStorage& assets, Assets::AssetUploads& uploads, Guid id)
        {
            const Assets::ShaderAsset* shader = assets.FindShader(id);
            if (!shader)
                return; // not in the storage: the renderer uses its default shader
            uploads.shaders.push_back({ .id = id, .revision = assets.ShaderRevision(id), .shader = *shader });
        }
    }

    namespace
    {
        /// @brief Adds a valid `id` to `ids` unless it is already there (a world names the same asset many times).
        void AddUniqueId(std::vector<Guid>& ids, Guid id)
        {
            if (id.IsValid() && std::ranges::find(ids, id) == ids.end())
                ids.push_back(id);
        }

        /// @brief Adds the texture `image` refers to; a render target or no image is not an asset.
        void AddRequiredTexture(Assets::RequiredAssets& required, const Common::ImageRef& image)
        {
            if (image.kind == Common::ImageRef::Kind::Texture)
                AddUniqueId(required.textures, image.id);
        }
    }

    Assets::RequiredAssets RequiredAssetsOf(const Render::RenderWorld& world)
    {
        Assets::RequiredAssets required;
        AddUniqueId(required.shaders, Render::DEFAULT_SCENE_SHADER);

        for (const Render::RenderObject& object : world.objects)
        {
            AddUniqueId(required.meshes, object.meshId);
            AddRequiredTexture(required, object.imageTexture);
            AddUniqueId(required.shaders, object.shaderId);
        }
        return required;
    }

    Assets::AssetUploads ProvideAssets(
        const Assets::AssetStorage& assets,
        const Assets::RequiredAssets& required,
        const Assets::ResidentAssets& resident)
    {
        Assets::AssetUploads uploads;

        // a mesh or texture the GPU lacks, or holds an older revision of
        for (const Guid id : required.meshes)
        {
            const Assets::ResidentAsset* onGpu = Find(resident.meshes, id);
            if (!onGpu || assets.MeshRevision(id) > onGpu->revision)
                AddMesh(assets, uploads, id);
        }
        for (const Guid id : required.textures)
        {
            const Assets::ResidentAsset* onGpu = Find(resident.textures, id);
            if (!onGpu || assets.TextureRevision(id) > onGpu->revision)
                AddTexture(assets, uploads, id);
        }

        // a shader the GPU has not seen, or one that did not compile at an older revision; compiled ones stay
        for (const Guid id : required.shaders)
        {
            if (Find(resident.shaders, id))
                continue;
            const Assets::ResidentAsset* failed = Find(resident.failedShaders, id);
            if (!failed || assets.ShaderRevision(id) > failed->revision)
                AddShader(assets, uploads, id);
        }

        return uploads;
    }
}

