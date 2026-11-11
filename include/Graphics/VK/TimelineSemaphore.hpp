/**
 * @file TimelineSemaphore.hpp
 * @brief A Vulkan timeline semaphore. one 64-bit counter the GPU raises and the CPU or GPU can wait on
 *
 * Used by the renderer for frame pacing (frame N signals value N + 1) and later by the render graph's FrameSubmitter
 * REQUIRES VkPhysicalDeviceVulkan12Features::timelineSemaphore.
 *  TODO: we only have a single queue, but keep in mind each one needs its own timeline
 */
#pragma once

#include <cstdint>
#include <limits>

#include <vulkan/vulkan_raii.hpp>

#include "Core/GlobalDefs.hpp"
#include "Graphics/VK/VulkanUtils.hpp"

namespace Vulkan
{
    /// @brief One monotonically increasing timeline
    /// @details The render graph's per-frame signal values are relative; Reserve() hands out the absolute base for
    /// a frame (values base + 1 .. base + count)
    class TimelineSemaphore
    {
    public:
        /// @param device must outlive the semaphore
        explicit TimelineSemaphore(const vk::raii::Device& device)
            : m_device(&device), m_semaphore(Make(device))
        {
        }

        /// @returns internal semaphore handle
        [[nodiscard]] vk::Semaphore Handle() const { return *m_semaphore; }

        /// @brief The highest value the GPU has signalled so far
        [[nodiscard]] TimelineValue Completed() const { return m_semaphore.getCounterValue(); }

        /// @returns true if GPU has signalled value or higher, false otherwise
        [[nodiscard]] bool IsComplete(TimelineValue value) const { return Completed() >= value; }

        /**
         * @brief Blocks the CPU until the GPU has signalled value.
         * @param value     value to wait for. 0 returns immediately (nothing has to be waited for)
         * @param timeoutNs nanoseconds before giving up, defaults to infinity (wait forever)
         * @throws std::runtime_error if the wait fails or times out
         */
        void Wait(TimelineValue value, std::uint64_t timeoutNs = std::numeric_limits<std::uint64_t>::infinity()) const
        {
            if (value == 0)
                return;

            const vk::Semaphore handle = *m_semaphore;
            const vk::SemaphoreWaitInfo info{.semaphoreCount = 1, .pSemaphores = &handle, .pValues = &value};

            Utils::CheckResult(m_device->waitSemaphores(info, timeoutNs), "Timeline semaphore wait failed");
        }

        /// @brief Reserves a number of values for one frame.
        /// @param count number of values for one frame
        /// @returns base values base + 1 .. base + count
        TimelineValue Reserve(std::uint32_t count)
        {
            const TimelineValue base = m_issued;
            m_issued += count;
            return base;
        }

        /// @returns The highest value that has been handed out by Reserve so far.
        [[nodiscard]] TimelineValue Issued() const { return m_issued; }

    private:
        /// @brief Constructor of 
        /// @param device must be destroyed after this
        static vk::raii::Semaphore Make(const vk::raii::Device& device)
        {
            // create timeline semaphore with default semaphore info
            vk::StructureChain<vk::SemaphoreCreateInfo, vk::SemaphoreTypeCreateInfo> chain{
                {},
                {
                    .semaphoreType = vk::SemaphoreType::eTimeline,
                    .initialValue = 0
                }
            };
            return vk::raii::Semaphore{device, chain.get<vk::SemaphoreCreateInfo>()};
        }

        const vk::raii::Device* m_device; ///< non-owning reference to device
        vk::raii::Semaphore m_semaphore; ///< owned semaphore
        TimelineValue m_issued = 0; ///< current timeline value
    };
}
