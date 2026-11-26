/**
 * @file RenderDevice.cpp
 * @brief Vulkan::RenderDevice implementation: queue setup, end of frame, missing assets.
 */
#include "Graphics/Vulkan/Frame/RenderDevice.hpp"

#include <array>
#include <print>

#include "Graphics/Vulkan/Core/Context.hpp"
#include "Graphics/Vulkan/Graph/RenderGraphVk.hpp"

namespace
{
    using namespace Droplet::Graphics::Vulkan;
    /// @brief The device queue of every logical render-graph queue; empty where the device has none of its own.
    std::array<FrameSubmitter::QueueSlot, RenderGraph::QUEUE_COUNT> MakeQueueSlots(
        const Context& context)
    {
        // A logical queue the device has no queue of its own for stays empty: the submitter sends its work to the
        // graphics queue, still with its own timeline.
        const QueueInfo& queueInfo = context.GetQueueInfo();
        std::array<FrameSubmitter::QueueSlot, RenderGraph::QUEUE_COUNT> queueSlots{};
      
        using Q = RenderGraph::Queue;
        for (const Q queue : {Q::Graphics, Q::Compute, Q::Transfer})
        {
            const QueueKind queueIndex = ToQueueIndex(queue);
            if (const vk::raii::Queue* deviceQueue = context.FindQueue(queueIndex))
            {
                queueSlots[RenderGraph::ToIndex(queue)] = {
                    .queue = deviceQueue,
                    .family = queueInfo.familyIndices[ToIndex(queueIndex)],
                    // the dedicated compute and transfer families are chosen without graphics (Device)
                    .canDraw = queue == Q::Graphics
                };
            }
        }
        return queueSlots;
    }
}

using namespace Droplet::Graphics::Vulkan;

RenderDevice::RenderDevice(const Context& context, std::uint32_t framesInFlight)
    : m_frameSubmitter(context.GetDevice(), MakeQueueSlots(context), framesInFlight, context.GetGpuProfiler()),
      m_textureTable(context, framesInFlight),
      m_resources(context, m_deletionQueue, m_textureTable),
      m_shaders(context.GetDevice())
{
}

void RenderDevice::BeginFrame()
{
    ZoneScoped
    m_resources.BeginFrame();
}

void RenderDevice::EndFrame()
{
    ZoneScoped
    // with no frame submitted, the last submitted frame's values still cover every use so far
    const QueueTimelineValues frameRetireValues = m_frameSubmitter.FrameRetireValues();

    const EvictedResources released = m_resources.EndFrame(frameRetireValues);
    if (released.Any())
    {
        std::println("RenderDevice: released {} mesh(es), {} texture(s), {} render target(s); replaced {}",
                     released.meshes, released.textures, released.targets, released.replaced);
        for (const ReleaseListener& listener : m_releaseListeners)
        {
            listener(frameRetireValues);
        }
    }

    m_deletionQueue.Collect(m_frameSubmitter.Completed());
}

void RenderDevice::Upload(const Bridge::ResourceUploads& uploads)
{
    ZoneScoped
    for (const Bridge::MeshUpload& mesh : uploads.meshes)
    {
        m_resources.UploadMesh(mesh);
    }
    for (const Bridge::TextureUpload& texture : uploads.textures)
    {
        m_resources.UploadTexture(texture);
    }
    for (const Bridge::ShaderUpload& shader : uploads.shaders)
    {
        m_shaders.Upload(shader); // reported once if it cannot be used
    }
}

Droplet::Bridge::ResidentResources RenderDevice::Residents() const
{
    ZoneScoped
    Bridge::ResidentResources resident;
    m_resources.AppendResident(resident);
    m_shaders.AppendResident(resident);
    return resident;
}

