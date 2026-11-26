/**
 * @file GpuProfiler.hpp
 * @brief Tracy GPU profiling: one Tracy Vulkan context per queue, and scoped GPU zones recorded into command buffers.
 */
#pragma once

#include <array>
#include <optional>
#include <source_location>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyVulkan.hpp>

#include "Graphics/Vulkan/Abstraction/Device.hpp"

namespace Droplet::Graphics::Vulkan
{
#if defined TRACY_ENABLE
    using GpuContext = tracy::VkCtx*; ///< Tracy Vulkan timeline, one per queue
#else
    using GpuContext = void*;
#endif

    /**
     * @brief Owns the Tracy Vulkan contexts for the device's queues and collects their timestamps.
     *
     * @details A queue kind the device has no dedicated queue for is served by the graphics context. A transfer queue cannot

     * Tracy contexts are not thread-safe: create and use zones from the main thread only.
     * Without TRACY_ENABLE every operation is a no-op.
     */
    class GpuProfiler
    {
    public:
        /// @brief Creates one Tracy context per dedicated queue.
        /// @param device the device whose queues are profiled; must outlive the profiler
        explicit GpuProfiler(const Device& device);
        ~GpuProfiler() noexcept;

        // no move no copy: Tracy contexts are referenced by pointer
        GpuProfiler(const GpuProfiler&) = delete;
        GpuProfiler& operator=(const GpuProfiler&) = delete;
        GpuProfiler(GpuProfiler&&) = delete;
        GpuProfiler& operator=(GpuProfiler&&) = delete;

        /// @brief Tracy context for queue kind, or null when Tracy is disabled.
        [[nodiscard]] GpuContext ForQueue(QueueKind kind) const noexcept;

        /// @brief Starts a frame: every context may be collected once again.
        void BeginFrame() const noexcept;

        /**
         * @brief Reads back finished GPU timestamps into Tracy and records the reset of their queries, once per frame
         *        per context
         * @param kind the queue `cmd` is recorded for
         * @param cmd a command buffer in recording state, outside any render pass, before its first zone
         */
        void Collect(QueueKind kind, vk::CommandBuffer cmd) const noexcept;

    private:
        /// A Tracy context this profiler created, with the pool its setup command buffer came from.
        struct OwnedContext
        {
            vk::raii::CommandPool pool{nullptr}; ///< Pool for the setup command buffer.
            GpuContext context{nullptr};         ///< Tracy context, destroyed in the dtor.
        };

        std::array<GpuContext, QUEUE_INDEX_COUNT> m_contexts{};  ///< Context per queue kind; shared when not dedicated.
        std::vector<OwnedContext> m_owned;                       ///< Contexts created by this profiler.
        mutable std::vector<GpuContext> m_collected;             ///< Contexts collected this frame (BeginFrame clears).
    };

    /**
     * @brief Scoped GPU timing zone: timestamps written into a command buffer at construction and destruction.
     *
     * @details Must be destroyed before the command buffer is ended. Non-copyable and non-movable.
     * The name is copied by Tracy. Without TRACY_ENABLE this does nothing.
     */
    class GpuZone
    {
    public:
        /**
         * @param profiler the profiler owning the queue's context
         * @param kind queue the command buffer is recorded for
         * @param cmd recording command buffer
         * @param name zone name shown in Tracy
         * @param location call site, shown as source location in Tracy
         */
        GpuZone(const GpuProfiler& profiler,
                QueueKind kind,
                vk::CommandBuffer cmd,
                const char* name,
                std::source_location location = std::source_location::current());
        ~GpuZone() noexcept = default;

        GpuZone(const GpuZone&) = delete;
        GpuZone& operator=(const GpuZone&) = delete;
        GpuZone(GpuZone&&) = delete;
        GpuZone& operator=(GpuZone&&) = delete;

    private:
#if defined TRACY_ENABLE
        std::optional<tracy::VkCtxScope> m_scope; ///< Empty when the queue has no Tracy context.
#endif
    };
}
