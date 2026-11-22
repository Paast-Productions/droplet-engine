/// @file GpuResourceManager.hpp
/// @brief Vulkan::GpuResourceManager: GPU meshes, textures and render targets by handle, released when unused.
#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>

#include "Integration/Core/Guid.hpp"
#include "Integration/Common/RenderDesc.hpp"

#include "Integration/Assets/AssetTransfer.hpp"
#include "Integration/Render/Handles.hpp"
#include "Integration/Vulkan/Core/ImageState.hpp"
#include "Integration/Vulkan/Resources/DeletionQueue.hpp"
#include "Integration/Vulkan/Resources/Uploader.hpp"
#include "Integration/Vulkan/Resources/RenderTarget.hpp"

namespace Vulkan
{
    class Context;

    /// @brief When a resource was last used, for eviction.
    struct Residency
    {
        std::uint64_t lastUsedFrame{0};
        QueueTimelineValues lastUsedRetireValues{}; ///< retire values of the last frame that used it
        bool pinned{false};                   ///< never evicted
        std::uint64_t revision{0};            ///< the asset storage's revision it was uploaded from; 0: not an asset
    };

    /// @brief A mesh on the GPU.
    struct GpuMesh
    {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        std::uint32_t vertexCount = 0;
        std::uint32_t indexCount = 0;
        vk::IndexType indexType = vk::IndexType::eUint32;
        Residency residency;
    };

    /// @brief A texture on the GPU.
    struct GpuTexture
    {
        Image image;
        ImageView view; ///< all mip levels; ShaderReadOnly after the upload
        vk::Extent2D extent{};
        vk::Sampler sampler{}; ///< from the texture asset's sampling settings; owned by the GpuResourceManager
        Residency residency;
        // With bindless, handle.index will double as the slot in the texture descriptor array
    };

    /// @brief A render target (or a target's history) on the GPU.
    struct GpuRenderTarget
    {
        RenderTarget target;
        /// Cleared to black when created and left ShaderReadOnly (a history: read before it is first written)
        bool clearedOnCreation{false};
        Common::RenderTargetDesc desc;
        Residency residency;
    };

    /// @brief How many resources one frame released: evicted by EndFrame, or replaced by a newer revision.
    struct EvictedResources
    {
        std::uint32_t meshes{0};
        std::uint32_t textures{0};
        std::uint32_t targets{0};  ///< including histories
        std::uint32_t replaced{0}; ///< meshes and textures replaced by a newer revision, targets by a new description

        [[nodiscard]] bool Any() const noexcept { return meshes + textures + targets + replaced > 0; }
    };

    /// @brief GPU resources by GUID: meshes and textures from uploads (Assets::AssetUploads), targets and histories
    ///        created on first request.
    /// @details Resolve* hands out the resident resource, or a placeholder (a cube, a blue and magenta checker) when
    ///          the asset is not on the GPU.
    ///          Every Resolve*, Find* by GUID and GetOrCreate* marks the resource used this frame. EndFrame releases
    ///          resources nobody asked for in `idleFramesBeforeEviction` frames, and an upload of a newer revision
    ///          replaces the resident one: either way the old one leaves the pools at once (its handle goes stale) and
    ///          is destroyed through the deletion queue once the GPU is past its last use.
    class GpuResourceManager
    {
    public:
        static constexpr std::uint32_t DEFAULT_IDLE_FRAMES_BEFORE_EVICTION = 60;

        /// @brief Creates the default sampler and the placeholder cube, placeholder texture and white texture.
        /// @param deletionQueue receives evicted and replaced resources; must outlive the manager
        GpuResourceManager(const Context& context, DeletionQueue& deletionQueue,
                                    std::uint32_t idleFramesBeforeEviction = DEFAULT_IDLE_FRAMES_BEFORE_EVICTION);

        /// @brief Starts a frame: Resolve*, Find* and GetOrCreate* calls from here on count as used in this frame.
        void BeginFrame();

        /// @brief Records this frame's retire values on what it used and retires what has been idle too long.
        /// @param frameRetireValues timeline values, per queue, after which the GPU is done with this frame
        /// @return what was evicted, and what was replaced since the last EndFrame
        EvictedResources EndFrame(const QueueTimelineValues& frameRetireValues);

        /// @brief Uploads the mesh unless the same or a newer revision is resident; an older one is replaced.
        /// @details A mesh whose vertex stride is not the scene layout's is not uploaded (reported once per revision):
        ///          a resident older revision is kept, else the placeholder cube is drawn. Returns the invalid handle.
        Render::MeshHandle UploadMesh(const Assets::MeshUpload& upload);
        /// @brief Uploads the texture (with its mip chain) unless the same or a newer revision is resident; an older
        ///        one is replaced.
        Render::TextureHandle UploadTexture(const Assets::TextureUpload& upload);

        /// @brief The mesh to draw for `id`, marked used: the resident one, or the placeholder cube.
        Render::MeshHandle ResolveMesh(Droplet::GUID id);
        /// @brief The texture to draw for `id`, marked used: the resident one, or the placeholder texture.
        Render::TextureHandle ResolveTexture(Droplet::GUID id);

        /// @brief Adds the resident meshes and textures, with the revisions they were uploaded from, to `resident`.
        void AppendResident(Assets::ResidentAssets& resident) const;

        /// @brief Drawn for a mesh that is not resident: a unit cube (Render::PLACEHOLDER_CUBE_HALF_EXTENT).
        [[nodiscard]] Render::MeshHandle PlaceholderMesh() const noexcept { return m_placeholderMesh; }
        /// @brief Drawn for a texture or render target that is not resident: a blue and magenta checker.
        [[nodiscard]] Render::TextureHandle PlaceholderTexture() const noexcept { return m_placeholderTexture; }
        /// @brief Drawn for an object that has no texture: 1 x 1 white.
        [[nodiscard]] Render::TextureHandle WhiteTexture() const noexcept { return m_whiteTexture; }

        /// @brief The resident mesh for `id`, marked used; invalid if it was never uploaded or has been evicted.
        Render::MeshHandle FindMesh(Droplet::GUID id);
        /// @brief The resident texture for `id`, marked used; invalid if it was never uploaded or has been evicted.
        Render::TextureHandle FindTexture(Droplet::GUID id);

        /// @brief The target for `desc`, marked used; created the first time, and replaced (new handle, new history)
        ///        when its size or format changed
        Render::TargetHandle GetOrCreateTarget(const Common::RenderTargetDesc& desc);
        /// @brief The existing target for `id`, marked used (it keeps its contents); invalid if there is none
        Render::TargetHandle FindTarget(Droplet::GUID id);
        Render::TargetHandle GetOrCreateHistory(Render::TargetHandle target);

        /// @brief A mesh without a GUID (debug or procedural geometry); never evicted, the caller releases it.
        Render::MeshHandle CreateDynamicMesh(
            std::span<const std::byte> vertexData, std::uint32_t vertexStride, std::span<const std::uint32_t> indices);
        /// @brief Releases a CreateDynamicMesh mesh once the GPU is past `retireValues`.
        void ReleaseDynamicMesh(Render::MeshHandle handle, const QueueTimelineValues& retireValues,
                                DeletionQueue& deletionQueue);


        /// @brief Keeps the texture resident for good, even if nobody asks for it.
        void Pin(Render::TextureHandle handle);

        /// @brief The resource behind `handle`, or nullptr if it is stale or invalid.
        const GpuMesh* Find(Render::MeshHandle handle) const { return m_meshes.Get(handle); }
        const GpuTexture* Find(Render::TextureHandle handle) const { return m_textures.Get(handle); }
        GpuRenderTarget* Find(Render::TargetHandle handle) { return m_targets.Get(handle); }
        const GpuRenderTarget* Find(Render::TargetHandle handle) const { return m_targets.Get(handle); }

        /// @brief The sampler for default texture settings (linear, repeat, no anisotropy); used for render targets.
        vk::Sampler DefaultSampler() const { return m_samplers.front().sampler; }

        /// @brief The sampler for `sampling`, created the first time; samplers are never destroyed (there are few).
        vk::Sampler GetOrCreateSampler(const Common::TextureSampling& sampling);

    private:
        /// @brief A mesh from vertex bytes in the scene layout (Vertex::P3N3UV2); not in the GUID lookup.
        Render::MeshHandle CreateMesh(std::span<const std::byte> vertexData, std::uint32_t vertexStride,
                                      std::span<const std::uint32_t> indices);
        /// @brief A texture (with its mip chain) from tightly packed RGBA8 pixels; not in the GUID lookup.
        Render::TextureHandle CreateTexture(std::uint32_t width, std::uint32_t height, Common::PixelFormat format,
                                            std::span<const std::byte> pixels, const Common::TextureSampling& sampling);
        Render::TargetHandle CreateTarget(const Common::RenderTargetDesc& desc);
        /// @brief Takes `target` and its history out of the pools, into the deletion queue, and reports them released
        void RetireTarget(Render::TargetHandle target);
        /// @brief Creates the placeholder cube, the placeholder checker texture and the white texture.
        void CreatePlaceholders();

        void MarkUsed(Residency& residency) const { residency.lastUsedFrame = m_frameIndex; }

        const Context* m_context;       ///< Not owned; outlives this.
        DeletionQueue* m_deletionQueue; ///< Not owned; outlives this.
        Uploader m_uploader;
        /// One per distinct sampling setting; the first is DefaultSampler().
        struct CachedSampler
        {
            Common::TextureSampling sampling;
            vk::raii::Sampler sampler;
        };
        std::vector<CachedSampler> m_samplers;
        std::uint32_t m_idleFramesBeforeEviction;
        std::uint64_t m_frameIndex{0};
        std::uint32_t m_replacedSinceEndFrame{0};

        HandlePool<Render::MeshTag, GpuMesh> m_meshes;
        HandlePool<Render::TextureTag, GpuTexture> m_textures;
        HandlePool<Render::TargetTag, GpuRenderTarget> m_targets;

        // only resources in a lookup are evicted (dynamic meshes are not in one)
        std::unordered_map<Droplet::GUID, Render::MeshHandle> m_meshLookup;
        std::unordered_map<Droplet::GUID, std::uint64_t> m_rejectedMeshes; ///< revision last reported as unusable
        std::unordered_map<Droplet::GUID, Render::TextureHandle> m_textureLookup;
        std::unordered_map<Droplet::GUID, Render::TargetHandle> m_targetLookup;
        std::unordered_map<std::uint64_t, Render::TargetHandle> m_historyLookup; ///< by the source target's Key()

        // == Placeholders == (no GUID, so never evicted)
        Render::MeshHandle m_placeholderMesh;
        Render::TextureHandle m_placeholderTexture;
        Render::TextureHandle m_whiteTexture;
    };
}
