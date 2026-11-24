/// @file DrawPass.hpp
/// @brief Vulkan::DrawPass: the scene pass. Draws one view's draw list with pipelines from the PipelineLibrary.
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <unordered_map>

#include "Core/Guid.hpp"
#include "Common/RenderData.hpp"
#include "Graphics/Render/RenderWorld.hpp"
#include "Graphics/Vulkan/Resources/DeletionQueue.hpp"
#include "Graphics/Vulkan/Resources/GpuResourceManager.hpp"
#include "Graphics/Vulkan/Resources/GraphicsPipeline.hpp"
#include "Graphics/Vulkan/Resources/PipelineLibrary.hpp"
#include "Graphics/Render/PreparedFrame.hpp"

namespace Droplet::Graphics::Vulkan
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
        /// @brief The built-in shader interface a dynamic mesh's vertex layout fits
        enum class MeshInterface : std::uint8_t
        {
            None,       ///< no built-in shader reads it
            Scene,      ///< position, normal, uv at locations 0, 1, 2: the scene shaders and materials
            VertexColor ///< position at 0, colour at 1: unlit, alpha blended
        };

        /// The renderer's shader for vertex-colour meshes (unlit), compiled on first use
        static constexpr GUID VERTEX_COLOR_SHADER = GuidFromName("shader/built-in-vertex-color");
        [[nodiscard]] static Common::ShaderDesc VertexColorShaderAsset();

        /// @brief Creates the layouts and the descriptor pool, and retires the sets of released textures and targets
        ///        whenever the device releases resources.
        /// @param context must outlive the pass
        /// @param device  must outlive the pass
        DrawPass(const Context& context, RenderDevice& device, const DrawPassCreateInfo& info);

        /// @brief The pipeline description for drawing with `shader` into the given attachment formats.
        [[nodiscard]] GraphicsPipelineDesc MakePipelineDesc(
            GUID shader, vk::Format colorFormat, vk::Format depthFormat, bool transparent = false) const;

        /**
         * @brief The built-in shader interface a mesh's vertex layout fits
         * @param mesh A dynamic mesh
         * @returns None if an attribute does not fit in the stride or no built-in shader reads the layout
         */
        [[nodiscard]] static MeshInterface InterfaceOf(const Common::DynamicMeshData& mesh);

        /**
         * @brief The pipeline description for a dynamic mesh: its own vertex layout, topology and depth test
         * @param shader The shader it draws with (VERTEX_COLOR_SHADER for MeshInterface::VertexColor)
         * @param mesh A mesh whose interface is not None
         * @param colorFormat The view's colour format
         * @param depthFormat The view's depth format
         * @param transparent Alpha blended without depth writes (vertex-colour meshes always blend)
         * @returns A description for PipelineLibrary::GetOrCreate
         */
        [[nodiscard]] GraphicsPipelineDesc MakeMeshPipelineDesc(
            GUID shader, const Common::DynamicMeshData& mesh,
            vk::Format colorFormat, vk::Format depthFormat, bool transparent) const;

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
