/// @file DrawPass.hpp
/// @brief Vulkan::DrawPass: the scene pass. Draws one view's draw list with pipelines from the PipelineLibrary.
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <vector>

#include "Core/Guid.hpp"
#include "Common/RenderData.hpp"
#include "Graphics/Render/PreparedFrame.hpp"
#include "Graphics/Vulkan/Abstraction/Buffer.hpp"
#include "Graphics/Render/RenderWorld.hpp"
#include "Graphics/Vulkan/Resources/DeletionQueue.hpp"
#include "Graphics/Vulkan/Resources/GpuResourceManager.hpp"
#include "Graphics/Vulkan/Resources/GraphicsPipeline.hpp"
#include "Graphics/Vulkan/Resources/PipelineLibrary.hpp"
#include "Graphics/Vulkan/Resources/TextureTable.hpp"

namespace Droplet::Graphics::Vulkan
{
    class Context;
    class RenderDevice;

    static_assert(sizeof(Common::PushConstantData) == 108, "the push constants game shaders declare");

    enum class DrawFilter
    {
        All,
        /// Leaves out draws that sample a render target (another camera's output or a history). Use until that
        /// camera has been rendered: sampling a target that has never been written is invalid
        SkipRenderTargetTextures,
    };

    struct DrawPassCreateInfo
    {
        std::uint32_t framesInFlight = 2; ///< one per-view uniform buffer per frame slot
    };

    /**
     * @brief The scene shader interface and the draw loop
     * @details Push constants: Graphics::PushConstantData (vertex and fragment). Set 0: the view's
     *          Graphics::Vulkan::ViewProjectionBuffer. Set 1: reserved. Set 2: the texture table, indexed by
     *          PushConstantData::textureID
     */
    class DrawPass
    {
    public:
        /// @brief The built-in shader interface a dynamic mesh's vertex layout fits
        enum class MeshInterface : std::uint8_t
        {
            None,       ///< no built-in shader reads it
            Scene,      ///< position, normal, uv at locations 0, 1, 2: the scene shaders and materials
            VertexColor, ///< position at 0, colour at 1: unlit, alpha blended
            Particle     ///< point list: centre, colour, size, rotation at 0-3; expanded to quads by the geometry shader
        };

        /// The renderer's shader for vertex-colour meshes (unlit), compiled on first use
        static constexpr GUID VERTEX_COLOR_SHADER = GuidFromName("shader/built-in-vertex-color");
        [[nodiscard]] static Common::ShaderDesc VertexColorShaderAsset();

        /**
         * @brief Creates the pipeline layout and the per-view uniform buffers
         * @param context Must outlive the pass
         * @param device Its resources and texture table; must outlive the pass
         * @param info The number of frame slots
         */
        DrawPass(const Context& context, RenderDevice& device, const DrawPassCreateInfo& info);

        /**
         * @brief The pipeline description for objects drawn with the scene layout
         * @param vertexShader The vertex stage's shader GUID
         * @param fragmentShader The fragment stage's shader GUID
         * @param colorFormat The view's colour format
         * @param depthFormat The view's depth format
         * @param transparent Alpha blended without depth writes
         * @returns A description for PipelineLibrary::GetOrCreate
         */
        [[nodiscard]] GraphicsPipelineDesc MakePipelineDesc(
			GUID vertexShader, GUID fragmentShader, GUID tessCtrlShader, GUID tessEvalShader, GUID geometryShader,
            vk::Format colorFormat, vk::Format depthFormat, bool transparent = false) const;

        /**
         * @brief The built-in shader interface a mesh's vertex layout fits
         * @param mesh A dynamic mesh
         * @returns None if an attribute does not fit in the stride or no built-in shader reads the layout
         */
        [[nodiscard]] static MeshInterface InterfaceOf(const Common::DynamicMeshData& mesh);

        /**
         * @brief The pipeline description for a dynamic mesh: its own vertex layout, topology and depth test
         * @param vertexShader The vertex stage's shader GUID
         * @param fragmentShader The fragment stage's shader GUID
         * @param mesh A mesh whose interface is not None
         * @param colorFormat The view's colour format
         * @param depthFormat The view's depth format
         * @param transparent Alpha blended without depth writes (vertex-colour meshes always blend)
         * @returns A description for PipelineLibrary::GetOrCreate
         */
        [[nodiscard]] GraphicsPipelineDesc MakeMeshPipelineDesc(
			GUID vertexShader, GUID fragmentShader, GUID tessCtrlShader, GUID tessEvalShader, GUID geometryShader,
            const Common::DynamicMeshData& mesh,
            vk::Format colorFormat, vk::Format depthFormat, bool transparent) const;

        /// @brief Writes every view's ViewProjectionBuffer and the frame's packed dynamic geometry. @pre the GPU is done with `frameSlot`'s last frame
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

    private:
        /// @brief The texture table index a draw samples; the placeholder's for a stale handle
        [[nodiscard]] std::uint32_t TextureIndexOf(const Render::ImageBinding& binding) const;

        const Context* m_context; ///< Not owned, outlives this.
        GpuResourceManager* m_gpu; ///< the device's; not owned
        TextureTable* m_textures;  ///< the device's; not owned

        vk::raii::DescriptorSetLayout m_viewSetLayout{nullptr};
        vk::raii::DescriptorSetLayout m_lightSetLayout{nullptr}; ///< replaced the reservedSetLayout
        //vk::raii::DescriptorSetLayout m_reservedSetLayout{nullptr}; ///< set 1: empty for now
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};

        // == Per-view uniforms == a buffer and a set per frame slot; the pool is declared before the sets
        vk::raii::DescriptorPool m_viewPool{nullptr};
        std::vector<vk::raii::DescriptorSet> m_viewSets;
        std::vector<Buffer> m_viewBuffers;
        
        // == Per-Frame lighting
        vk::raii::DescriptorPool m_lightPool{nullptr};
        std::vector<vk::raii::DescriptorSet> m_lightSets;
        std::vector<Buffer> m_lightBuffers;

        // == Dynamic geometry == rewritten every frame, a vertex and an index buffer per frame slot
        std::vector<Buffer> m_dynamicVertexBuffers;
        std::vector<Buffer> m_dynamicIndexBuffers;
        
        vk::DeviceSize m_viewUniformStride = 0; ///< sizeof(ViewProjectionBuffer), aligned for dynamic offsets
        std::uint32_t m_frameSlot = 0;
        std::uint32_t m_lightCount = 0;
        float m_time = 0.0f; ///< the frame's PreparedFrame::time
    };
}
