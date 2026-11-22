/// @file DrawPass.hpp
/// @brief Vulkan::DrawPass: the scene pass. Draws one view's draw list with pipelines from the PipelineLibrary.
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <unordered_map>

#include "Integration/Render/RenderWorld.hpp"
#include "Integration/Core/Guid.hpp"
#include "Integration/Vulkan/Resources/DeletionQueue.hpp"
#include "Integration/Vulkan/Resources/GpuResourceManager.hpp"
#include "Integration/Vulkan/Resources/GraphicsPipeline.hpp"
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"
#include "Integration/Render/PreparedFrame.hpp"

namespace Vulkan
{
    class Context;
    class RenderDevice;

    /// Per-draw shader data, sent as push constants (vertex stage). 128 bytes is the push-constant size every Vulkan
    /// device guarantees, so no limit query is needed.
    struct ScenePushConstants
    {
        glm::mat4 viewProj; ///< projection * view of the camera being rendered
        glm::mat4 model;    ///< object to world
    };
    static_assert(sizeof(ScenePushConstants) == 128, "must fit the guaranteed 128 bytes of push constants");

    /// Per-view shader data at set 1 binding 0 (vertex and fragment stages)
    struct SceneViewUniforms
    {
        glm::vec4 cameraPosition{ 0.0f }; ///< world space; w unused
        glm::vec4 cameraForward{ 0.0f };  ///< world space, unit length; w unused
        float time = 0.0f;                ///< seconds
        float padding[3]{};
    };

    enum class DrawFilter
    {
        All,
        /// Leaves out draws that sample a render target (another camera's output or a history). Use until that
        /// camera has been rendered: sampling a target that has never been written is invalid
        SkipRenderTargetTextures,
    };

    struct DrawPassCreateInfo
    {
        std::uint32_t maxMaterials = 64; ///< descriptor pool size: distinct textures and targets drawn with at once
        std::uint32_t framesInFlight = 2; ///< one per-view uniform buffer per frame slot
    };

    /// @brief The scene material interface (vertex layout P3N3UV2, set 0 binding 0 base colour, set 1 binding 0
    ///        SceneViewUniforms, ScenePushConstants) and the draw loop. Material sets are cached per texture and target; an unresolved or stale binding draws
    ///        the placeholder texture.
    class DrawPass
    {
    public:
        /// The renderer's shader for P3C4 dynamic geometry (vertex colour, unlit), compiled on first use
        static constexpr Guid VERTEX_COLOR_SHADER = Guid::FromName("shader/built-in-vertex-color");
        [[nodiscard]] static Common::ShaderDesc VertexColorShaderAsset();

        /// @brief Creates the layouts and the descriptor pool, and retires the sets of released textures and targets
        ///        whenever the device releases resources.
        /// @param context must outlive the pass
        /// @param device  must outlive the pass
        DrawPass(const Context& context, RenderDevice& device, const DrawPassCreateInfo& info);

        /// @brief The pipeline description for drawing with `shader` into the given attachment formats.
        [[nodiscard]] GraphicsPipelineDesc MakePipelineDesc(
            Droplet::GUID shader, vk::Format colorFormat, vk::Format depthFormat, bool transparent = false) const;

        /// @brief The pipeline description for dynamic geometry of `layout` (P3C4 ignores `transparent`: it always blends)
        [[nodiscard]] GraphicsPipelineDesc MakeDynamicPipelineDesc(
            Droplet::GUID shader, Render::DynamicVertexLayout layout, Render::DynamicTopology topology,
            vk::Format colorFormat, vk::Format depthFormat, bool transparent, bool depthTest) const;

        /// @brief Writes every view's SceneViewUniforms and the frame's packed dynamic geometry. @pre the GPU is done with `frameSlot`'s last frame
        void BeginFrame(std::uint32_t frameSlot, const Render::PreparedFrame& frame);

        /// @brief Records `view`'s draws inside an open dynamic-rendering scope; sets viewport (Y flipped) and scissor.
        /// @param viewIndex the list's position in the frame given to BeginFrame (its uniforms)
        void Record(
            vk::CommandBuffer cmd,
            vk::Extent2D extent,
            const Render::DrawList& list,
            std::uint32_t viewIndex,
            const PipelineLibrary& pipelines,
            DrawFilter filter = DrawFilter::All);

        /// @brief Moves the cached sets of evicted textures and targets into the deletion queue.
        /// @param frameRetireValues timeline values after which this frame is done (its draws may use them)
        void RetireDeadMaterials(DeletionQueue& deletionQueue, const QueueTimelineValues& frameRetireValues);

        /// @brief Forgets every cached set. @pre the GPU is idle
        void ClearMaterialCache();

    private:
        vk::DescriptorSet MaterialSet(const Render::ImageBinding& binding);
        vk::raii::DescriptorSet CreateSet(vk::ImageView view, vk::Sampler sampler) const;

        const Context* m_context; ///< Not owned, outlives this.
        GpuResourceManager* m_gpu; ///< the device's; not owned

        vk::raii::DescriptorSetLayout m_setLayout{nullptr};
        vk::raii::DescriptorSetLayout m_viewSetLayout{nullptr};
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};

        // == Per-view uniforms == a buffer and a set per frame slot; the pool is declared before the sets
        vk::raii::DescriptorPool m_viewPool{nullptr};
        std::vector<vk::raii::DescriptorSet> m_viewSets;
        std::vector<Buffer> m_viewBuffers;

        // == Dynamic geometry == rewritten every frame, a vertex and an index buffer per frame slot
        std::vector<Buffer> m_dynamicVertexBuffers;
        std::vector<Buffer> m_dynamicIndexBuffers;
        vk::DeviceSize m_viewUniformStride = 0; ///< sizeof(SceneViewUniforms), aligned for dynamic offsets
        std::uint32_t m_frameSlot = 0;

        // The pool is declared before the sets so the sets are destroyed first.
        vk::raii::DescriptorPool m_pool{nullptr};
        std::unordered_map<std::uint64_t, vk::raii::DescriptorSet> m_textureSets;
        std::unordered_map<std::uint64_t, vk::raii::DescriptorSet> m_targetSets;
    };
}
