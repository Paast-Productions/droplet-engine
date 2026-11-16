/// @file DeletionQueue.hpp
/// @brief Vulkan::DeletionQueue: keeps retired GPU objects alive until the GPU has passed their last use.
#pragma once

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "Vulkan/Core/QueueTimelineValues.hpp"

namespace Vulkan
{
    /// @brief Objects waiting to be destroyed, each with the timeline values after which the GPU no longer uses it.
    /// @details One value per queue (FrameSubmitter::FrameRetireValues / Completed): a frame's work is spread over
    ///          the graphics, compute and transfer queues.
    class DeletionQueue
    {
    public:
        DeletionQueue() = default;
        ~DeletionQueue() = default;

        DeletionQueue(const DeletionQueue&) = delete;
        DeletionQueue& operator=(const DeletionQueue&) = delete;

        /// @brief Takes `object` and destroys it once Collect sees every queue at or past `retireValues`.
        template <class T>
        void Retire(T&& object, const QueueTimelineValues& retireValues)
        {
            m_entries.push_back({retireValues, std::make_shared<std::decay_t<T>>(std::forward<T>(object))});
        }

        /// @brief Destroys every object the GPU is done with on every queue.
        void Collect(const QueueTimelineValues& completedValues)
        {
            std::erase_if(m_entries, [&completedValues](const Entry& entry)
            {
                return entry.retireValues.ReachedBy(completedValues);
            });
        }

        /// @brief Destroys everything. @pre the GPU is idle
        void Flush() { m_entries.clear(); }

        [[nodiscard]] std::size_t Size() const noexcept { return m_entries.size(); }

    private:
        struct Entry
        {
            QueueTimelineValues retireValues{};
            std::shared_ptr<void> object; ///< type-erased owner; destroying it destroys the object
        };

        std::vector<Entry> m_entries;
    };
}
