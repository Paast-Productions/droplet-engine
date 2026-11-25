/**
 * @file Semaphore.cpp
 * @brief Vulkan::Semaphore implementation.
 */
#include "Graphics/Vulkan/Abstraction/Semaphore.hpp"

#include <stdexcept>
#include <string>

#include "Graphics/Vulkan/Core/VulkanUtils.hpp"

namespace Droplet::Graphics::Vulkan
{
    Semaphore::Semaphore(const vk::raii::Device& device, Type type, TimelineValue initialValue)
        : m_device(&device),
          m_type(type),
          m_issued(initialValue)
    {
        const vk::SemaphoreTypeCreateInfo typeInfo{
            .semaphoreType = type == Type::Timeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary,
            .initialValue = type == Type::Timeline ? initialValue : 0
        };
        m_semaphore = vk::raii::Semaphore{device, vk::SemaphoreCreateInfo{.pNext = &typeInfo}};
    }

    vk::SemaphoreSubmitInfo Semaphore::SubmitInfo(vk::PipelineStageFlags2 stages, TimelineValue value) const
    {
        return {
            .semaphore = *m_semaphore,
            .value = IsTimeline() ? value : 0,
            .stageMask = stages
        };
    }

    TimelineValue Semaphore::Completed() const
    {
        RequireTimeline("Completed");
        return m_semaphore.getCounterValue();
    }

    void Semaphore::Wait(TimelineValue value, std::uint64_t timeoutNs) const
    {
        RequireTimeline("Wait");
        if (value == 0)
            return;
        const vk::Semaphore handle = *m_semaphore;
        const vk::SemaphoreWaitInfo waitInfo{.semaphoreCount = 1, .pSemaphores = &handle, .pValues = &value};
        Utils::CheckResult(m_device->waitSemaphores(waitInfo, timeoutNs), "Timeline semaphore wait failed");
    }

    TimelineValue Semaphore::Issued() const
    {
        RequireTimeline("Issued");
        return m_issued;
    }

    TimelineValue Semaphore::Reserve(std::uint32_t count)
    {
        RequireTimeline("Reserve");
        const TimelineValue base = m_issued;
        m_issued += count;
        return base;
    }

    void Semaphore::RequireTimeline(const char* operation) const
    {
        if (!IsTimeline())
            throw std::logic_error(std::string("Semaphore::") + operation + " called on a binary semaphore");
    }
}
