/**
 * @file GpuProfiler.cpp
 * @brief Tracy GPU profiling implementation.
 */
#include "Graphics/Vulkan/Core/GpuProfiler.hpp"

#include <cstring>

namespace
{
    using Droplet::Graphics::Vulkan::GpuContext;

    /**
     * @brief Creates a Tracy context on queue, recording its setup command buffer from a new pool.
     * @param device the logical device
     * @param queue queue to time on
     * @param family queue family of queue
     * @param name name shown for the context in Tracy
     * @param pool receives the pool the setup command buffer came from; must outlive the context
     */
    GpuContext CreateContext(const Droplet::Graphics::Vulkan::Device& device,
                             [[maybe_unused]] vk::Queue queue,
                             uint32_t family,
                             [[maybe_unused]] const char* name,
                             vk::raii::CommandPool& pool)
    {
        pool = vk::raii::CommandPool{
            device.Get(),
            vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                      .queueFamilyIndex = family}};
        const vk::raii::CommandBuffers commandBuffers{
            device.Get(),
            vk::CommandBufferAllocateInfo{.commandPool = *pool,
                                          .level = vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1}};
        [[maybe_unused]] const vk::CommandBuffer setupCommandBuffer = *commandBuffers.front();

        GpuContext context = TracyVkContext(*device.GetPhysicalDevice(), *device.Get(), queue, setupCommandBuffer);
        TracyVkContextName(context, name, std::strlen(name));
        return context;
    }
}

namespace Droplet::Graphics::Vulkan
{
    GpuProfiler::GpuProfiler(const Device& device)
    {
        m_owned.reserve(QUEUE_INDEX_COUNT);
        const QueueInfo& info = device.GetQueueInfo();

        const GpuContext graphics = [&]
        {
            vk::raii::CommandPool pool{nullptr};
            const GpuContext context = CreateContext(device,
                                                     *device.GetGraphicsQueue(),
                                                     info.familyIndices[ToIndex(QueueKind::Graphics)],
                                                     "Graphics",
                                                     pool);
            m_owned.push_back(OwnedContext{std::move(pool), context});
            return context;
        }();
        m_contexts[ToIndex(QueueKind::Graphics)] = graphics;

        for (const QueueKind kind : {QueueKind::Compute, QueueKind::Transfer})
        {
            const vk::raii::Queue* queue = device.FindQueue(kind);
            if (!queue)
            {
                m_contexts[ToIndex(kind)] = graphics;
                continue;
            }

            vk::raii::CommandPool pool{nullptr};
            const GpuContext context = CreateContext(device,
                                                     **queue,
                                                     info.familyIndices[ToIndex(kind)],
                                                     ToString(kind),
                                                     pool);
            m_owned.push_back(OwnedContext{std::move(pool), context});
            m_contexts[ToIndex(kind)] = context;
        }
    }

    GpuProfiler::~GpuProfiler() noexcept
    {
        for ([[maybe_unused]] const OwnedContext& owned : m_owned)
        {
            TracyVkDestroy(owned.context);
        }
    }

    GpuContext GpuProfiler::ForQueue(QueueKind kind) const noexcept
    {
        return m_contexts[ToIndex(kind)];
    }

    void GpuProfiler::Collect() const noexcept
    {
        for ([[maybe_unused]] const OwnedContext& owned : m_owned)
        {
            TracyVkCollect(owned.context, VK_NULL_HANDLE);
        }
    }

    GpuZone::GpuZone([[maybe_unused]] const GpuProfiler& profiler,
                     [[maybe_unused]] QueueKind kind,
                     [[maybe_unused]] vk::CommandBuffer cmd,
                     [[maybe_unused]] const char* name,
                     [[maybe_unused]] std::source_location location)
    {
#if defined TRACY_ENABLE
        if (const GpuContext context = profiler.ForQueue(kind))
        {
            m_scope.emplace(context,
                            location.line(),
                            location.file_name(),
                            std::strlen(location.file_name()),
                            location.function_name(),
                            std::strlen(location.function_name()),
                            name,
                            std::strlen(name),
                            static_cast<VkCommandBuffer>(cmd),
                            true);
        }
#endif
    }
}
