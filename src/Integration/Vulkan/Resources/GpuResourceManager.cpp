/// @file GpuResourceManager.cpp
/// @brief Vulkan::GpuResourceManager implementation: creation, upload and eviction.
#include "Integration/Vulkan/Resources/GpuResourceManager.hpp"

#include "Integration/Vulkan/Core/Context.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <optional>
#include <print>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Integration/Render/RenderWorld.hpp"
#include "Integration/Vulkan/Resources/MeshVertex.hpp"

namespace Vulkan
{
    // Common::MeshVertex is uploaded as-is, so it has to match the vertex layout on the GPU
    static_assert(sizeof(Common::MeshVertex) == sizeof(Vertex::P3N3UV2));
    static_assert(offsetof(Common::MeshVertex, position) == offsetof(Vertex::P3N3UV2, position));
    static_assert(offsetof(Common::MeshVertex, normal) == offsetof(Vertex::P3N3UV2, normal));
    static_assert(offsetof(Common::MeshVertex, uv) == offsetof(Vertex::P3N3UV2, uv));

    namespace
    {
        vk::Format ToVk(Common::PixelFormat format)
        {
            switch (format)
            {
            case Common::PixelFormat::RGBA8_SRGB: return vk::Format::eR8G8B8A8Srgb;
            case Common::PixelFormat::RGBA8_UNORM: return vk::Format::eR8G8B8A8Unorm;
            }
            throw std::invalid_argument("Unknown or unsupported PixelFormat");
        }

        /// @brief A cube of edge 2 * halfExtent around the origin: four vertices per face with the face's normal,
        ///        uv (0,0) at each face's top-left, counter-clockwise seen from outside (as the spoof scene's cubes).
        void MakeCube(float halfExtent, std::vector<Common::MeshVertex>& vertices, std::vector<std::uint32_t>& indices)
        {
            struct Face
            {
                glm::vec3 normal, right, up;
            };
            static constexpr std::array<Face, 6> faces{ {
                {.normal = {1.f, 0.f, 0.f},  .right = {0.f, 0.f, -1.f}, .up = {0.f, 1.f, 0.f}},
                {.normal = {-1.f, 0.f, 0.f}, .right = {0.f, 0.f, 1.f},  .up = {0.f, 1.f, 0.f}},
                {.normal = {0.f, 1.f, 0.f},  .right = {1.f, 0.f, 0.f},  .up = {0.f, 0.f, -1.f}},
                {.normal = {0.f, -1.f, 0.f}, .right = {1.f, 0.f, 0.f},  .up = {0.f, 0.f, 1.f}},
                {.normal = {0.f, 0.f, 1.f},  .right = {1.f, 0.f, 0.f},  .up = {0.f, 1.f, 0.f}},
                {.normal = {0.f, 0.f, -1.f}, .right = {-1.f, 0.f, 0.f}, .up = {0.f, 1.f, 0.f}},
            } };
            for (const Face& face : faces)
            {
                const auto base = static_cast<std::uint32_t>(vertices.size());
                const glm::vec3 center = face.normal * halfExtent;
                const glm::vec3 right = face.right * halfExtent;
                const glm::vec3 up = face.up * halfExtent;
                vertices.push_back({ .position = center - right - up, .normal = face.normal, .uv = {0.0f, 1.0f} });
                vertices.push_back({ .position = center + right - up, .normal = face.normal, .uv = {1.0f, 1.0f} });
                vertices.push_back({ .position = center + right + up, .normal = face.normal, .uv = {1.0f, 0.0f} });
                vertices.push_back({ .position = center - right + up, .normal = face.normal, .uv = {0.0f, 0.0f} });
                for (const std::uint32_t corner : {0u, 1u, 2u, 0u, 2u, 3u})
                {
                    indices.push_back(base + corner);
                }
            }
        }

        /// @brief Tightly packed RGBA8 pixels of a `size` x `size` checker with `cells` x `cells` squares.
        std::vector<std::byte> MakeChecker(std::uint32_t size, std::uint32_t cells,
                                           std::array<std::uint8_t, 4> first, std::array<std::uint8_t, 4> second)
        {
            std::vector<std::byte> pixels;
            pixels.reserve(std::size_t{ size } * size * 4);
            const std::uint32_t cellSize = std::max(1u, size / cells);
            for (std::uint32_t row = 0; row < size; ++row)
            {
                for (std::uint32_t column = 0; column < size; ++column)
                {
                    const bool isFirst = ((row / cellSize) + (column / cellSize)) % 2 == 0;
                    for (const std::uint8_t channel : isFirst ? first : second)
                    {
                        pixels.push_back(std::byte{ channel });
                    }
                }
            }
            return pixels;
        }

        /// @brief Stamps this frame's retire values on what it used and moves what has been idle for `idleFrames`
        ///        frames out of `pool` and `lookup` into the deletion queue.
        /// @return how many were evicted
        template <class Key, class Tag, class T>
        std::uint32_t EvictIdle(
            std::unordered_map<Key, Handle<Tag>>& lookup,
            HandlePool<Tag, T>& pool,
            std::uint64_t frameIndex,
            std::uint32_t idleFrames,
            const QueueTimelineValues& frameRetireValues,
            DeletionQueue& deletionQueue)
        {
            std::uint32_t evicted = 0;
            for (auto entryIt = lookup.begin(); entryIt != lookup.end();)
            {
                T* resource = pool.Get(entryIt->second);
                if (!resource)
                {
                    entryIt = lookup.erase(entryIt);
                    continue;
                }

                Residency& residency = resource->residency;
                if (residency.lastUsedFrame == frameIndex)
                    residency.lastUsedRetireValues = frameRetireValues;

                if (!residency.pinned && frameIndex - residency.lastUsedFrame >= idleFrames)
                {
                    const QueueTimelineValues retireValues = residency.lastUsedRetireValues;
                    deletionQueue.Retire(std::move(*pool.Remove(entryIt->second)), retireValues);
                    entryIt = lookup.erase(entryIt);
                    ++evicted;
                    continue;
                }
                ++entryIt;
            }
            return evicted;
        }
    }

    GpuResourceManager::GpuResourceManager(
        const Context& context, DeletionQueue& deletionQueue, std::uint32_t idleFramesBeforeEviction)
        : m_context(&context),
          m_deletionQueue(&deletionQueue),
          m_uploader(context),
          m_idleFramesBeforeEviction(idleFramesBeforeEviction)
    {
        GetOrCreateSampler(Common::TextureSampling{}); // DefaultSampler()
        CreatePlaceholders();
    }

    void GpuResourceManager::CreatePlaceholders()
    {
        // a cube with the checker on every face: obviously not the real asset, drawable by the default scene shader
        std::vector<Common::MeshVertex> vertices;
        std::vector<std::uint32_t> indices;
        MakeCube(Render::PLACEHOLDER_CUBE_HALF_EXTENT, vertices, indices);
        m_placeholderMesh = CreateMesh(std::as_bytes(std::span(vertices)), sizeof(Common::MeshVertex), indices);

        constexpr std::array<std::uint8_t, 4> BLUE{ 30, 60, 255, 255 };
        constexpr std::array<std::uint8_t, 4> MAGENTA{ 255, 0, 255, 255 };
        const std::vector<std::byte> checker = MakeChecker(64, 8, BLUE, MAGENTA);
        m_placeholderTexture = CreateTexture(64, 64, Common::PixelFormat::RGBA8_SRGB, checker,
                                             Common::TextureSampling{ .filter = Common::TextureFilter::Nearest });

        // objects that have no texture at all
        constexpr std::array<std::byte, 4> whitePixel{
            std::byte{ 0xFF }, std::byte{ 0xFF }, std::byte{ 0xFF }, std::byte{ 0xFF } };
        m_whiteTexture = CreateTexture(1, 1, Common::PixelFormat::RGBA8_UNORM, whitePixel,
                                       Common::TextureSampling{ .generateMips = false });
    }

    void GpuResourceManager::BeginFrame()
    {
        ++m_frameIndex;
    }

    vk::Sampler GpuResourceManager::GetOrCreateSampler(const Common::TextureSampling& sampling)
    {
        // generateMips does not change the sampler: maxLod is unclamped, so it works with any number of levels
        const auto sameFilteringAs = [&sampling](const CachedSampler& cached)
        {
            return cached.sampling.filter == sampling.filter && cached.sampling.repeat == sampling.repeat
                && cached.sampling.maxAnisotropy == sampling.maxAnisotropy;
        };
        if (const auto cachedIt = std::ranges::find_if(m_samplers, sameFilteringAs); cachedIt != m_samplers.end())
            return *cachedIt->sampler;

        const bool nearest = sampling.filter == Common::TextureFilter::Nearest;
        const vk::Filter filter = nearest ? vk::Filter::eNearest : vk::Filter::eLinear;
        const vk::SamplerAddressMode addressMode =
            sampling.repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge;
        const float anisotropy = std::clamp(sampling.maxAnisotropy, 1.0f, m_context->GetMaxSamplerAnisotropy());

        m_samplers.push_back({
            .sampling = sampling,
            .sampler = vk::raii::Sampler(m_context->GetDevice(), vk::SamplerCreateInfo{
                .magFilter = filter,
                .minFilter = filter,
                .mipmapMode = nearest ? vk::SamplerMipmapMode::eNearest : vk::SamplerMipmapMode::eLinear,
                .addressModeU = addressMode,
                .addressModeV = addressMode,
                .addressModeW = addressMode,
                .anisotropyEnable = anisotropy > 1.0f ? vk::True : vk::False,
                .maxAnisotropy = anisotropy,
                .maxLod = vk::LodClampNone,
                .borderColor = vk::BorderColor::eIntOpaqueBlack
            })
        });
        return *m_samplers.back().sampler;
    }

    EvictedResources GpuResourceManager::EndFrame(const QueueTimelineValues& frameRetireValues)
    {
        DeletionQueue& deletionQueue = *m_deletionQueue;
        EvictedResources evicted;
        evicted.replaced = std::exchange(m_replacedSinceEndFrame, 0u);
        evicted.meshes = EvictIdle(m_meshLookup, m_meshes, m_frameIndex, m_idleFramesBeforeEviction,
                                   frameRetireValues, deletionQueue);
        evicted.textures = EvictIdle(m_textureLookup, m_textures, m_frameIndex, m_idleFramesBeforeEviction,
                                     frameRetireValues, deletionQueue);
        evicted.targets = EvictIdle(m_targetLookup, m_targets, m_frameIndex, m_idleFramesBeforeEviction,
                                    frameRetireValues, deletionQueue)
                        + EvictIdle(m_historyLookup, m_targets, m_frameIndex, m_idleFramesBeforeEviction,
                                    frameRetireValues, deletionQueue);
        return evicted;
    }

    void GpuResourceManager::Pin(Render::TextureHandle handle)
    {
        if (GpuTexture* texture = m_textures.Get(handle))
            texture->residency.pinned = true;
    }

    Render::MeshHandle GpuResourceManager::CreateMesh(
        std::span<const std::byte> vertexData, std::uint32_t vertexStride, std::span<const std::uint32_t> indices)
    {
        if (vertexData.empty() || indices.empty())
            throw std::invalid_argument("Mesh has no vertices or indices");
        if (vertexStride != sizeof(Vertex::P3N3UV2) || vertexData.size() % vertexStride != 0)
            throw std::invalid_argument("Mesh vertices are not in the scene layout (Vertex::P3N3UV2)");

        GpuMesh gpu;
        gpu.vertexCount = static_cast<std::uint32_t>(vertexData.size() / vertexStride);
        gpu.indexCount = static_cast<std::uint32_t>(indices.size());
        MarkUsed(gpu.residency);
        gpu.vertexBuffer = m_uploader.CreateDeviceBuffer(
            vertexData.data(),
            vertexData.size_bytes(),
            vk::BufferUsageFlagBits::eVertexBuffer);
        gpu.indexBuffer = m_uploader.CreateDeviceBuffer(
            indices.data(),
            indices.size_bytes(),
            vk::BufferUsageFlagBits::eIndexBuffer);
        return m_meshes.Add(std::move(gpu));
    }

    Render::TargetHandle GpuResourceManager::CreateTarget(const Common::RenderTargetDesc& desc)
    {
        GpuRenderTarget gpu;
        gpu.desc = desc;
        MarkUsed(gpu.residency);
        gpu.target = RenderTarget::Create(
            m_context->GetAllocator(), m_context->GetPhysicalDevice(), m_context->GetDevice(),
            RenderTargetCreateInfo{
                .image = {
                    .extent = {.width = desc.width, .height = desc.height},
                    .format = ToVk(desc.format),
                    .usage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eSampled |
                    vk::ImageUsageFlagBits::eTransferSrc | // copied into its history
                    vk::ImageUsageFlagBits::eTransferDst,  // is a history
                    // drawn on the graphics queue, copied on the transfer queue (the render graph does no
                    // queue-family ownership transfers)
                    .sharedQueueFamilies = m_context->GetQueueInfo().UniqueFamilies()
                }
            });
        // Stays Undefined: the render graph's first pass that touches it transitions it.
        // Sampling it before it has ever been rendered is invalid, so the graph must order producer first.
        return m_targets.Add(std::move(gpu));
    }

    Render::MeshHandle GpuResourceManager::FindMesh(Guid id)
    {
        const auto cachedMesh = m_meshLookup.find(id);
        if (cachedMesh == m_meshLookup.end())
            return {};
        GpuMesh* existing = m_meshes.Get(cachedMesh->second);
        if (!existing)
            return {};
        MarkUsed(existing->residency);
        return cachedMesh->second;
    }

    Render::TextureHandle GpuResourceManager::FindTexture(Guid id)
    {
        const auto cachedTexture = m_textureLookup.find(id);
        if (cachedTexture == m_textureLookup.end())
            return {};
        GpuTexture* existing = m_textures.Get(cachedTexture->second);
        if (!existing)
            return {};
        MarkUsed(existing->residency);
        return cachedTexture->second;
    }

    Render::MeshHandle GpuResourceManager::UploadMesh(const Assets::MeshUpload& upload)
    {
        // the scene pipelines read one vertex layout; any other is reported once per revision and not uploaded, and a
        // resident older revision keeps being drawn
        if (upload.vertexStride != sizeof(Vertex::P3N3UV2) || upload.vertexData.size() % upload.vertexStride != 0)
        {
            if (const auto rejectedIt = m_rejectedMeshes.find(upload.id);
                rejectedIt == m_rejectedMeshes.end() || rejectedIt->second < upload.revision)
            {
                std::println("GpuResourceManager: mesh {:016x} has a vertex stride of {} bytes ({} bytes of data); the scene "
                             "layout needs {}. Not uploaded", upload.id.value, upload.vertexStride,
                             upload.vertexData.size(), sizeof(Vertex::P3N3UV2));
                m_rejectedMeshes.insert_or_assign(upload.id, upload.revision);
            }
            return {};
        }

        if (const auto residentIt = m_meshLookup.find(upload.id); residentIt != m_meshLookup.end())
        {
            if (GpuMesh* resident = m_meshes.Get(residentIt->second))
            {
                if (resident->residency.revision >= upload.revision)
                    return residentIt->second;

                // an older revision: it leaves the pool now and is destroyed once the GPU is past its last use (uploads
                // run before anything is resolved this frame, so this frame does not use it)
                const QueueTimelineValues retireValues = resident->residency.lastUsedRetireValues;
                m_deletionQueue->Retire(std::move(*m_meshes.Remove(residentIt->second)), retireValues);
                ++m_replacedSinceEndFrame;
            }
        }

        const Render::MeshHandle handle = CreateMesh(upload.vertexData, upload.vertexStride, upload.indices);
        m_meshes.Get(handle)->residency.revision = upload.revision;
        m_meshLookup.insert_or_assign(upload.id, handle);
        return handle;
    }

    Render::TextureHandle GpuResourceManager::UploadTexture(const Assets::TextureUpload& upload)
    {
        if (const auto residentIt = m_textureLookup.find(upload.id); residentIt != m_textureLookup.end())
        {
            if (GpuTexture* resident = m_textures.Get(residentIt->second))
            {
                if (resident->residency.revision >= upload.revision)
                    return residentIt->second;

                // as for meshes; DrawPass retires the old texture's material set when it hears of the release
                const QueueTimelineValues retireValues = resident->residency.lastUsedRetireValues;
                m_deletionQueue->Retire(std::move(*m_textures.Remove(residentIt->second)), retireValues);
                ++m_replacedSinceEndFrame;
            }
        }

        const Render::TextureHandle handle =
            CreateTexture(upload.width, upload.height, upload.format, upload.pixels, upload.sampling);
        m_textures.Get(handle)->residency.revision = upload.revision;
        m_textureLookup.insert_or_assign(upload.id, handle);
        return handle;
    }

    Render::TextureHandle GpuResourceManager::CreateTexture(
        std::uint32_t width, std::uint32_t height, Common::PixelFormat format,
        std::span<const std::byte> pixels, const Common::TextureSampling& sampling)
    {
        GpuTexture gpu;
        MarkUsed(gpu.residency);
        gpu.extent = vk::Extent2D{
            .width = width,
            .height = height
        };
        // a full chain: every halving down to 1 x 1
        const std::uint32_t mipLevels = sampling.generateMips ? std::bit_width(std::max(width, height)) : 1u;
        gpu.image = m_uploader.UploadImage2D(pixels.data(), pixels.size(), gpu.extent, ToVk(format), mipLevels);
        gpu.view = ImageView::Create2D(m_context->GetDevice(), gpu.image,
                                       ImageViewCreateOptions{
                                           .aspectMask = vk::ImageAspectFlagBits::eColor,
                                           .levelCount = gpu.image.MipLevels()});
        gpu.sampler = GetOrCreateSampler(sampling);
        return m_textures.Add(std::move(gpu));
    }

    Render::MeshHandle GpuResourceManager::ResolveMesh(Guid id)
    {
        const Render::MeshHandle resident = FindMesh(id);
        return m_meshes.Get(resident) ? resident : m_placeholderMesh;
    }

    Render::TextureHandle GpuResourceManager::ResolveTexture(Guid id)
    {
        const Render::TextureHandle resident = FindTexture(id);
        return m_textures.Get(resident) ? resident : m_placeholderTexture;
    }

    void GpuResourceManager::AppendResident(Assets::ResidentAssets& resident) const
    {
        for (const auto& [id, handle] : m_meshLookup)
        {
            if (const GpuMesh* mesh = m_meshes.Get(handle))
                resident.meshes.push_back({ .id = id, .revision = mesh->residency.revision });
        }
        for (const auto& [id, handle] : m_textureLookup)
        {
            if (const GpuTexture* texture = m_textures.Get(handle))
                resident.textures.push_back({ .id = id, .revision = texture->residency.revision });
        }
    }

    Render::MeshHandle GpuResourceManager::CreateDynamicMesh(
        std::span<const std::byte> vertexData, std::uint32_t vertexStride, std::span<const std::uint32_t> indices)
    {
        return CreateMesh(vertexData, vertexStride, indices);
    }

    void GpuResourceManager::ReleaseDynamicMesh(
        Render::MeshHandle handle, const QueueTimelineValues& retireValues, DeletionQueue& deletionQueue)
    {
        if (std::optional<GpuMesh> mesh = m_meshes.Remove(handle))
            deletionQueue.Retire(std::move(*mesh), retireValues);
    }

    Render::TargetHandle GpuResourceManager::GetOrCreateTarget(const Common::RenderTargetDesc& desc)
    {
        if (const auto cachedTarget = m_targetLookup.find(desc.id); cachedTarget != m_targetLookup.end())
        {
            if (GpuRenderTarget* existing = m_targets.Get(cachedTarget->second))
            {
                MarkUsed(existing->residency);
                return cachedTarget->second;
            }
        }

        const Render::TargetHandle handle = CreateTarget(desc);
        m_targetLookup.insert_or_assign(desc.id, handle);
        return handle;
    }

    Render::TargetHandle GpuResourceManager::GetOrCreateHistory(Render::TargetHandle target)
    {
        const std::uint64_t targetKey = target.Key();
        if (const auto cachedHistory = m_historyLookup.find(targetKey); cachedHistory != m_historyLookup.end())
        {
            if (GpuRenderTarget* existing = m_targets.Get(cachedHistory->second))
            {
                MarkUsed(existing->residency);
                return cachedHistory->second;
            }
        }

        const GpuRenderTarget* source = Find(target);
        if (!source)
        {
            return {};
        }

        // history keeps the source's description, including guid
        // history targets are only available through history lookup, never target lookup, so that's fine
        const Render::TargetHandle history = CreateTarget(source->desc);

        // a history is read (as last frame's image) before anything has written it: start it black, not undefined
        GpuRenderTarget* created = m_targets.Get(history);
        m_uploader.ClearToSampled(created->target.GetImage(),
                                  created->target.GetImageResource().GetSubresourceRange(vk::ImageAspectFlagBits::eColor));
        created->clearedOnCreation = true;
        m_historyLookup.insert_or_assign(targetKey, history);
        return history;
    }
}
