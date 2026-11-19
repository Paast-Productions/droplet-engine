/**
 * @file FrameSubmitter.hpp
 * @brief Vulkan::FrameSubmitter: records a compiled render graph, one command buffer and one vkQueueSubmit2 per graph
 *        submission, synchronised with one timeline semaphore per queue (Route B).
 */
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Core/Timeline.hpp"
#include "Integration/Vulkan/Core/QueueTimelineValues.hpp"
#include "Integration/Vulkan/Abstraction/Semaphore.hpp"
#include "Integration/RenderGraph/RenderGraph.hpp"
#include "Integration/Vulkan/Graph/RenderGraphVk.hpp"

namespace Vulkan
{
    /// @brief Submits render graphs with one timeline per logical queue: BeginFrame (wait for the slot), Prepare
    ///        (reserve timeline values), graph.Execute (one command buffer and submit2 per Submission), EndFrame.
    /// @details A logical queue without a device queue submits to the graphics queue and keeps its own timeline, so
    ///          a single-queue device runs any graph. Not copyable or movable.
    /**
     *@brief Submits render graphs with one timeline per logical queue
     *@details 
     *  Usage:
     *     BeginFrame : wait for the frame slot to be available on the GPU
     *     Prepare    : prepare to execute the graph by storing data and reserving timeline values
     *     RenderGraph::Execute : executes and submits the compiled render graph
     *     EndFrame   : Mark this frame as finished
     */
    class FrameSubmitter
    {
    public:
        /// @brief A physical queue and its family, for one logical RenderGraph::Queue.
        struct QueueSlot
        {
            const vk::raii::Queue* queue = nullptr; ///< null: use the graphics queue
            std::uint32_t family = 0;               ///< family index of queue
            bool canDraw = false;                   ///< the family supports graphics (barriers may name any access)
        };

        /// @brief Binary semaphores of imports synchronised outside the graph (ImportOptions::waitExternal/signalExternal)
        struct External
        {
            /// Semaphore to wait on before the first use of the resource (the swapchain image-available semaphore).
            std::function<const Semaphore&(RenderGraph::ResourceId)> acquire;
            /// Semaphore to signal after the last use of the resource (render-finished, waited on by present).
            std::function<const Semaphore&(RenderGraph::ResourceId)> finished;
        };

        /**
         * @brief Creates one timeline per logical queue and command pools per frame slot and queue.
         * @param device         the device; must outlive the submitter
         * @param queues         physical queue per logical queue; the graphics entry is required
         * @param framesInFlight number of frame slots
         * @throws std::invalid_argument if there is no graphics queue
         */
        FrameSubmitter(const vk::raii::Device& device, std::array<QueueSlot, RenderGraph::QUEUE_COUNT> queues,
                       std::uint32_t framesInFlight);
        ~FrameSubmitter() = default;

        // no copy, no move
        FrameSubmitter(const FrameSubmitter&) = delete;
        FrameSubmitter& operator=(const FrameSubmitter&) = delete;
        FrameSubmitter(FrameSubmitter&&) = delete;
        FrameSubmitter& operator=(FrameSubmitter&&) = delete;

        /**
         * @brief Waits for the GPU to finish the slot's previous frame on every queue, then resets its command pools
         * @param frameSlot in [0, framesInFlight)
         * @throws std::runtime_error if a wait fails
         */
        void BeginFrame(std::uint32_t frameSlot);

        /**
         * @brief Reserves this frame's timeline values; call once per frame, before graph.Execute(compiled, *this).
         * @param compiled  the compiled graph about to be executed; must stay alive until EndFrame
         * @param resources this frame's physical images; must stay alive until EndFrame; handed to pass callbacks
         * @param external  semaphores of the graph's external waits and signals
         */
        void Prepare(const RenderGraph::Compiled& compiled, const ResourceTable& resources, External external);

        /**
         * @brief Makes a submission of the prepared frame also wait for earlier work the graph does not know about:
         *        the last use, on another queue, of a pooled image one of its transients was given.
         * @param submissionIndex index into the prepared Compiled::submissions
         * @param carryWait       queue and absolute timeline value to wait for
         * @pre between Prepare and Graph::Execute
         */
        void AddCarryWait(std::uint32_t submissionIndex, RenderGraph::CarryWait carryWait);

        // ---- Recorder interface (RenderGraph::Graph::Execute) ----

        /// @brief Opens a command buffer for `submission` from the slot's pool of its queue.
        PassContext& Begin(const RenderGraph::Submission& submission);

        /// @brief Records `barriers` (also used for the epilogue) into the open command buffer.
        void Barriers(PassContext& context, std::span<const RenderGraph::Barrier> barriers);

        /// @brief Closes the command buffer and submits it with the waits and signals `submission` describes.
        void End(const PassContext& context, const RenderGraph::Submission& submission);

        /**
         * @brief Ends the frame: the values Prepare reserved become what BeginFrame waits for next time this slot is
         *        used, and the compiled graph and resource table are no longer referenced.
         */
        void EndFrame();

        /// @brief The absolute timeline value of signal `signalIndex` (1-based, per queue) of the current frame.
        [[nodiscard]] TimelineValue Absolute(RenderGraph::Queue queue, std::uint32_t signalIndex) const;

        /// @brief Values the GPU has reached on every queue; anything retired at or below them is safe to destroy
        ///        or reuse.
        [[nodiscard]] QueueTimelineValues Completed() const;

        /// @brief Values the current frame reaches on every queue once all its submissions are done (after Prepare).
        [[nodiscard]] const QueueTimelineValues& FrameRetireValues() const noexcept { return m_frameRetire; }

        /// @brief The timeline semaphore of `queue`.
        [[nodiscard]] const Semaphore& Timeline(RenderGraph::Queue queue) const;

    private:
        /// @brief What one frame slot owns; reused once the GPU is done with the slot's previous frame
        struct FrameSlot
        {
            /// one transient pool per logical queue: a pool serves one queue family, and is reset as a whole
            std::array<vk::raii::CommandPool, RenderGraph::QUEUE_COUNT> commandPools;
            /// recorded this frame from commandPools; declared after them, so destroyed first
            std::vector<vk::raii::CommandBuffer> commandBuffers;
            /// the values to wait for, on every queue, before the slot is reused
            QueueTimelineValues retireValues{};
        };

        const vk::raii::Device* m_device;
        std::array<QueueSlot, RenderGraph::QUEUE_COUNT> m_queues;
        std::array<Semaphore, RenderGraph::QUEUE_COUNT> m_timelines; ///< Type::Timeline
        std::vector<FrameSlot> m_slots;                              ///< one per frame in flight


        std::uint32_t m_frameSlot = 0;
        std::array<TimelineValue, RenderGraph::QUEUE_COUNT> m_frameBase{};   ///< Prepare's reservation, per queue
        QueueTimelineValues m_frameRetire{};                                 ///< base + signals, per queue
        std::vector<std::vector<RenderGraph::CarryWait>> m_extraCarryWaits;  ///< AddCarryWait, per submission
        const RenderGraph::Compiled* m_compiled = nullptr;                   ///< Between Prepare and EndFrame
        const ResourceTable* m_resources = nullptr;                          ///< Between Prepare and EndFrame
        External m_external;
        PassContext m_passContext;
    };

    /**
     * @brief Where an imported resource was left, including the ABSOLUTE timeline value of its last access.
     *
     * Store it and pass it back to Graph::Import next frame: that is what makes waits across frames and queues work.
     * @pre `submitter` has prepared the frame `compiled` was executed in (Absolute uses its reservation)
     */
    RenderGraph::ResourceState FinalState(
        const FrameSubmitter& submitter,
        const RenderGraph::GraphData& graph,
        const RenderGraph::Compiled& compiled,
        RenderGraph::ResourceId resource);
}
