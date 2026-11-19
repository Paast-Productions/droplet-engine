/// @file QueueTimelineValues.hpp
/// @brief Vulkan::QueueTimelineValues: one timeline value per logical render-graph queue.
#pragma once

#include <array>
#include <cstddef>

#include "Integration/Core/Timeline.hpp"
#include "Integration/RenderGraph/RenderGraphTypes.hpp"

namespace Vulkan
{
    /// @brief A point on every queue's timeline (FrameSubmitter): what a frame signals there, or what the GPU has
    ///        completed. A frame's work can be on several queues, so "the GPU is done with it" needs all of them.
    struct QueueTimelineValues
    {
        std::array<TimelineValue, RenderGraph::QUEUE_COUNT> values{};

        [[nodiscard]] TimelineValue& operator[](RenderGraph::Queue queue) noexcept
        {
            return values[RenderGraph::ToIndex(queue)];
        }

        [[nodiscard]] TimelineValue operator[](RenderGraph::Queue queue) const noexcept
        {
            return values[RenderGraph::ToIndex(queue)];
        }

        /// @brief True if `completed` is at or past these values on every queue.
        [[nodiscard]] bool ReachedBy(const QueueTimelineValues& completed) const noexcept
        {
            for (std::size_t queueIndex = 0; queueIndex < values.size(); ++queueIndex)
            {
                if (values[queueIndex] > completed.values[queueIndex])
                    return false;
            }
            return true;
        }
    };
}
