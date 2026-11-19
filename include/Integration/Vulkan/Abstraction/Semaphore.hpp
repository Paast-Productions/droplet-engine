/**
 * @file Semaphore.hpp
 * @brief Abstraction of a Vulkan semaphore that can be either binary or timeline, used for synchronization
 */
#pragma once

#include <cstdint>
#include <limits>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Core/Timeline.hpp"

namespace Vulkan
{
    /**
    * @brief A Vulkan semaphore created as either binary or timeline.
    * @details Timelines are created per queue used, binaries are used when acquiring the swapchain and presenting.
    * As a timeline semaphore, the semaphore only increments its stored 64 bit unsigned integer. A fence or barrier
    * then waits for the timeline value to be >= a set value.
     */

    class Semaphore
    {
    public:
        enum class Type
        {
            Binary, ///< signalled and waited on once per use (swapchain acquire and present)
            Timeline ///< a 64-bit counter the GPU raises; the CPU and GPU wait for values
        };
        
        /// @param device MUST outlive the semaphore
        /// @param initialValue timeline only
        Semaphore(const vk::raii::Device& device, Type type, TimelineValue initialValue = 0);

        Semaphore(Semaphore&&) noexcept = default;
        Semaphore& operator=(Semaphore&&) noexcept = default;
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        ///@brief Get the vulkan handle of the stored semaphore, not the raii instance
        [[nodiscard]] vk::Semaphore Handle() const noexcept { return *m_semaphore; }
        ///@brief stored type enum, timeline or binary 
        [[nodiscard]] Type GetType() const noexcept { return m_type; }
        ///@brief true if semaphore is timeline, false if binary
        [[nodiscard]] bool IsTimeline() const noexcept { return m_type == Type::Timeline; }

        /// @brief How a submit waits on / signals the semaphore
        /// @param stages the stage that needs to be signalled
        /// @param value the value to wait for. Ignored for binary semaphore
        /// @returns a vk::SemaphoreSubmitInfo to be used by the queue submit2
        [[nodiscard]] vk::SemaphoreSubmitInfo SubmitInfo(vk::PipelineStageFlags2 stages, TimelineValue value = 0) const;

        /// @brief The highest value the GPU has signalled. @throws std::logic_error on a binary semaphore
        [[nodiscard]] TimelineValue Completed() const;

        /// @brief True if the GPU has signalled `value` (or a later one). @throws std::logic_error on a binary semaphore
        [[nodiscard]] bool IsComplete(TimelineValue value) const { return Completed() >= value; }

        /// @brief The highest value handed out by Reserve. @throws std::logic_error on a binary semaphore
        [[nodiscard]] TimelineValue Issued() const;

        /// @brief Blocks the CPU until the GPU has signalled `value`; 0 returns at once.
        /// @throws std::logic_error on a binary semaphore; std::runtime_error if the wait fails or times out
        void Wait(TimelineValue value, std::uint64_t timeoutNs = std::numeric_limits<std::uint64_t>::max()) const;

        /// @brief Reserves `count` values for one frame and returns the base (values base + 1 .. base + count).
        /// @throws std::logic_error on a binary semaphore
        TimelineValue Reserve(std::uint32_t count);

    private:
        /// @throws std::logic_error naming `operation` if this is a binary semaphore
        void RequireTimeline(const char* operation) const;

        const vk::raii::Device* m_device;
        Type m_type;
        vk::raii::Semaphore m_semaphore{nullptr};
        TimelineValue m_issued{0}; ///< timeline only: the highest value handed out by Reserve
    };
}
