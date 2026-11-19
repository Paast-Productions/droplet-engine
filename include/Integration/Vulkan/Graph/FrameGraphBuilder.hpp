/**
 * @file FrameGraphBuilder.hpp
 * @brief Builds the frame's render graph from its Render::PreparedFrame. 
 * Vulkan layer: the graph's passes receive a PassContext.
 */
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Integration/Common/RenderDesc.hpp"
#include "Integration/Render/Handles.hpp"
#include "Integration/Render/PreparedFrame.hpp"
#include "Integration/RenderGraph/RenderGraph.hpp"
#include "Integration/Vulkan/Graph/RenderGraphVk.hpp"
#include "Integration/Vulkan/Passes/PostProcessShader.hpp"

namespace Vulkan
{
    using FrameGraph = RenderGraph::Graph<PassContext>;
    
    /// turn a render target handle into a key
    inline std::uint64_t TargetKey(Render::TargetHandle handle)
    {
        return handle.Key();
    }
    /// get a render target handle from a key
    inline Render::TargetHandle TargetFromKey(std::uint64_t key)
    {
        return Render::TargetHandle::FromKey(key);
    }

    /// External key of the swapchain image import. Its index half is Handle's invalid index, so no target key can be it
    inline constexpr std::uint64_t SWAPCHAIN_KEY = ~std::uint64_t{0};

    /// ids for graph resources one view pass renders into
    struct ViewTargets
    {
        RenderGraph::ResourceId color{}; // id of the color target
        RenderGraph::ResourceId depth{}; // id of the depth target
    };

    /// @brief How the frame ends: the composite of the presented view into the swapchain image.
    struct PresentationInputs
    {
        bool enabled{false}; ///< false: no swapchain import, no composite
        /// false: the composite only clears, so nothing reads the presented view for it (the editor shows views in
        /// its windows instead)
        bool compositeScene{true};
    };

    /// @brief Which kind of pass renders an effect step.
    enum class EffectTechnique
    {
        Fragment, ///< fullscreen triangle on the graphics queue (Compositor); one output pixel from one input pixel
        Compute   ///< compute shader on the compute queue (EffectPass); effects that read many pixels
    };

    /// @brief How a step uses its effect's buffer (a transient storage buffer, STEP_BUFFER_SIZE bytes, that carries
    ///        a result from one step of the effect to a later one). Compute steps only.
    enum class StepBufferUse
    {
        None,
        Writes, ///< creates the buffer
        Reads
    };

    /// @brief Size of an effect's step buffer in bytes.
    inline constexpr std::uint64_t STEP_BUFFER_SIZE{16};

    /// @brief One pass of an effect.
    struct EffectStep
    {
        PostProcess::Effect effect{PostProcess::Effect::Passthrough}; ///< the shader code it runs
        float amount{1.0f};
        EffectTechnique technique{EffectTechnique::Fragment};
        bool writesImage{true}; ///< false: the step only writes its buffer; the next step reads the same image
        StepBufferUse buffer{StepBufferUse::None};
    };

    /// @brief The passes a scene-side effect (Common::PostEffect) is rendered with, in order. The technique is a fixed
    ///        property of the effect: per-pixel colour effects are fragment passes, the blur is two compute passes,
    ///        and auto exposure is a compute pass that measures the image into a buffer and one that applies it.
    [[nodiscard]] std::vector<EffectStep> StepsFor(const Common::PostEffectDesc& effect);

    /// What goes into a frame graph
    struct FrameGraphInputs
    {
        std::function<RenderGraph::ResourceState(Render::TargetHandle)> currentState; ///< State of each target after last frame (default first time)  GraphExecutor stores FinalState(...) of every import after executing (GraphExecutor::ImportState).
        std::function<bool(Render::TargetHandle)> isDisplayed; ///< true is something out side the graph consumes the target and keeps it alive (i.e. imgui viewport): Present view does not need it.
        std::uint32_t depthFormat{0}; ///< format for a view's depth transient vk::Format. 0 means no depth
        std::function<std::uint32_t(Render::TargetHandle)> targetFormat; ///< vk::Format of a target. Effects draws onto a transient with target format
        std::uint32_t effectFormat{0}; ///< storage image vk::Format of the images a compute effect writes. 0 means they are skipped

        /// Records the draws for one view (begin/end rendering, bind, draw). Called in graph order.
        std::function<void(PassContext&, const Render::DrawList&, const ViewTargets&)> drawView;
        /**
         * @brief Copies one render target into another of the same size and format
         * Called for the history pass that refreshes a Render::RenderView's history target
         * @param src Target to copy from, has TransferSrc layout set
         * @param dst Target to copy into, has TransferDst layout set
         * @note graph records the barriers for src and dst; recorded on the transfer queue
         */
        std::function<void(PassContext&, Render::TargetHandle src, Render::TargetHandle dst)> copyTarget;

        /**
         * @brief Applies one fragment step of `view`'s effect chain: samples `source`, writes all of `destination`
         *        (the view's target, or a transient of its format).
         * @note the graph has `source` in SampledRead and `destination` in ColorWrite; recorded on the graphics queue
         */
        std::function<void(PassContext&, const Render::DrawList& view, const EffectStep& step,
                           RenderGraph::ResourceId source, RenderGraph::ResourceId destination)> fragmentEffect;

        /**
         * @brief Applies one compute step of `view`'s effect chain: samples `source`, writes all of `destination`
         *        (a transient of `effectFormat`) and/or uses the effect's step buffer.
         * @param destination invalid if the step writes no image (EffectStep::writesImage)
         * @param stepBuffer  the effect's step buffer (a transient buffer of STEP_BUFFER_SIZE bytes);
         *                    invalid if the step does not use it
         * @note the graph has `source` in SampledRead, `destination` in StorageWrite and `stepBuffer` in StorageWrite
         *       (EffectStep::buffer Writes) or StorageRead (Reads); recorded on the compute queue
         */
        std::function<void(PassContext&, const Render::DrawList& view, const EffectStep& step,
                           RenderGraph::ResourceId source, RenderGraph::ResourceId destination,
                           RenderGraph::ResourceId stepBuffer)> computeEffect;

        /**
         * @brief Writes the end of `view`'s effect chain into the view's target, when the chain ends in a compute
         *        step: samples `source`, writes all of `destination`.
         * @note the graph has `source` in SampledRead and `destination` in ColorWrite; recorded on the graphics queue
         */
        std::function<void(PassContext&, const Render::DrawList& view,
                           RenderGraph::ResourceId source, RenderGraph::ResourceId destination)> resolveEffects;

        /// Composite; see PresentationInputs.
        PresentationInputs presentation{};
        /**
         * @brief Writes the final image into the swapchain image `destination`, stretched to its size.
         * @param source the presented view's target (after its effects); invalid when nothing is presented, then the
         *               composite only clears
         * @note the graph moves `destination` to Present afterwards (its required final usage)
         */
        std::function<void(PassContext&, RenderGraph::ResourceId source,
                           RenderGraph::ResourceId destination)> composite;

        /**
         * @brief Draws on top of the composite into the swapchain image `destination` (the editor's ImGui). Unset: no
         *        overlay pass.
         * @note the graph has `destination` in ColorWrite and `overlayReads` in SampledRead; graphics queue
         */
        std::function<void(PassContext&, RenderGraph::ResourceId destination)> overlay;
        std::vector<Render::TargetHandle> overlayReads; ///< render targets the overlay samples (ImGui windows)
    };

    /// Pre-compiled frame graph build containing the graph, id for the swapchain target and any warnings generated during build
    struct FrameGraphBuild
    {
        FrameGraph graph;
        std::vector<std::string> warnings;
        /// The swapchain image import (SWAPCHAIN_KEY); invalid when presentation is disabled.
        RenderGraph::ResourceId swapchain{};
    };
    
    /**
     * @brief Build a frame graph containing one graph pass per view (draw list)
     * @details Each view's read become Read() edges to make camera-as-texture ordering and barriers come out of the graph.
     * A view with PostProcess effect will draw into a transient image and place the final image in the view's target.
     * Any reader will get the final image with effects.
     * History copies run on the transfer queue. If a view has presentation enabled it draws right to the swapchain,
     * else it draws to its target.
     * World must survive until the compiled graph has been executed as the pass will contain callbacks to its data.
     */
    FrameGraphBuild BuildFrameGraph(const Render::PreparedFrame& frame, const FrameGraphInputs& inputs);
}
