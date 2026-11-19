/**
 * @file GraphExecutor.hpp
 * @brief Vulkan::GraphExecutor: runs a compiled render graph on the GPU (physical images, barriers, pass callbacks)
 *        and remembers what each image was left as for the next frame.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Core/Timeline.hpp"
#include "Integration/Vulkan/Frame/FrameSubmitter.hpp"
#include "Integration/Vulkan/Resources/TransientPool.hpp"
#include "Integration/RenderGraph/RenderGraph.hpp"
#include "Integration/Vulkan/Graph/RenderGraphVk.hpp"

namespace Vulkan
{
    class Context;
    class RenderDevice;

    /// @brief The frame a graph is executed in.
    struct GraphExecutionInfo
    {
        FrameSubmitter* submitter{nullptr};    ///< Records and submits; BeginFrame was called for this frame slot.
        FrameSubmitter::External external{};   ///< Semaphores of the graph's external waits and signals (swapchain).
        vk::Extent2D swapchainExtent{};        ///< What SwapchainRelative transients scale.
    };

    /// @brief Executes compiled render graphs: physical images and buffers for every live resource, recording and
    ///        submission through the FrameSubmitter, and the state each import and pooled resource is left in for the
    ///        next frame (docs/frame_flow.md 3.3). Not copyable or movable.
    class GraphExecutor
    {
    public:
        /// @brief The physical image or buffer behind an imported resource (identified by ResourceInfo::externalKey).
        using ResolveImportFn = std::function<PhysicalResource(const RenderGraph::ResourceInfo& resource)>;

        /**
         * @brief Creates the transient pool; forgets the import states of render targets the device releases.
         * @param context device and allocator; must outlive the executor
         * @param device  must outlive the executor
         */
        GraphExecutor(const Context& context, RenderDevice& device);
        ~GraphExecutor() = default;

        // no copy, no move
        GraphExecutor(const GraphExecutor&) = delete;
        GraphExecutor& operator=(const GraphExecutor&) = delete;
        GraphExecutor(GraphExecutor&&) = delete;
        GraphExecutor& operator=(GraphExecutor&&) = delete;

        /**
         * @brief Where the import with `externalKey` was left by the last graph that used it (imports with
         *        ImportOptions::waitExternal, like the swapchain image, are not remembered).
         * @return its final usage, queue and absolute timeline value; all-default (Usage::None) if no graph used it
         */
        [[nodiscard]] RenderGraph::ResourceState ImportState(std::uint64_t externalKey) const;

        /// @brief Forgets the import states whose key `isGone` returns true for (released render targets).
        void ForgetImports(const std::function<bool(std::uint64_t externalKey)>& isGone);

        /**
         * @brief Records and submits a compiled graph (FrameSubmitter::Prepare, Graph::Execute, FrameSubmitter::EndFrame).
         * @param graph         the graph `compiled` was compiled from; its pass callbacks receive a PassContext
         * @param compiled      result of graph.Compile(); must be ok
         * @param executionInfo the frame submitter, the external semaphores and the swapchain size
         * @param resolveImport physical image of an import; called once per live import
         * @pre FrameSubmitter::BeginFrame was called for this frame slot (TransientPool relies on the wait)
         * @throws std::logic_error if `compiled` failed or there is no submitter
         */
        void Execute(
            const RenderGraph::Graph<PassContext>& graph,
            const RenderGraph::Compiled& compiled,
            const GraphExecutionInfo& executionInfo,
            const ResolveImportFn& resolveImport);

    private:
        TransientPool m_transientPool; ///< Physical images for transients, reused across frames.
        /// Final state of every import a graph has used, by external key; next frame's initial state.
        std::unordered_map<std::uint64_t, RenderGraph::ResourceState> m_importStates;
    };
}
