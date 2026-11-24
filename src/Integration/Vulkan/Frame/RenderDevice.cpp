/**
 * @file RenderDevice.cpp
 * @brief Vulkan::RenderDevice implementation: queue setup, end of frame, missing assets.
 */
#include "Integration/Vulkan/Frame/RenderDevice.hpp"

#include <array>
#include <print>

#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Graph/RenderGraphVk.hpp"

namespace
{
    /// @brief The device queue of every logical render-graph queue; empty where the device has none of its own.
    std::array<Vulkan::FrameSubmitter::QueueSlot, RenderGraph::QUEUE_COUNT> MakeQueueSlots(
        const Vulkan::Context& context)
    {
        // A logical queue the device has no queue of its own for stays empty: the submitter sends its work to the
        // graphics queue, still with its own timeline.
        const Vulkan::QueueInfo& queueInfo = context.GetQueueInfo();
        std::array<Vulkan::FrameSubmitter::QueueSlot, RenderGraph::QUEUE_COUNT> queueSlots{};
        for (const RenderGraph::Queue queue :
             {RenderGraph::Queue::Graphics, RenderGraph::Queue::Compute, RenderGraph::Queue::Transfer})
        {
            const Vulkan::QueueKind queueIndex = Vulkan::ToQueueIndex(queue);
            if (const vk::raii::Queue* deviceQueue = context.FindQueue(queueIndex))
                queueSlots[RenderGraph::ToIndex(queue)] = {
                    .queue = deviceQueue,
                    .family = queueInfo.familyIndices[Vulkan::ToIndex(queueIndex)],
                    // the dedicated compute and transfer families are chosen without graphics (Device)
                    .canDraw = queue == RenderGraph::Queue::Graphics
                };
        }
        return queueSlots;
    }
}

using namespace Vulkan;

RenderDevice::RenderDevice(const Context& context, std::uint32_t framesInFlight)
    : m_frameSubmitter(context.GetDevice(), MakeQueueSlots(context), framesInFlight),
      m_resources(context, m_deletionQueue),
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

void RenderDevice::Upload(const Assets::AssetUploads& uploads)
{
    ZoneScoped
    for (const Assets::MeshUpload& mesh : uploads.meshes)
    {
        m_resources.UploadMesh(mesh);
    }
    for (const Assets::TextureUpload& texture : uploads.textures)
    {
        m_resources.UploadTexture(texture);
    }
    for (const Assets::ShaderUpload& shader : uploads.shaders)
    {
        m_shaders.GetOrCreate(shader.id, shader.shader, shader.revision); // reported once if it does not compile
    }
}

Assets::ResidentAssets RenderDevice::Resident() const
{
    ZoneScoped
    Assets::ResidentAssets resident;
    m_resources.AppendResident(resident);
    m_shaders.AppendResident(resident);
    return resident;
}

