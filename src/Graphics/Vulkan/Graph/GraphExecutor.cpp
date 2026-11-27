/**
 * @file GraphExecutor.cpp
 * @brief Vulkan::GraphExecutor implementation: physical images, execution through the FrameSubmitter, and the state
 *        carried to the next frame.
 */
#include "Graphics/Vulkan/Graph/GraphExecutor.hpp"

#include <bit>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Graphics/Vulkan/Core/VulkanUtils.hpp"
#include "Graphics/Vulkan/Frame/RenderDevice.hpp"
#include "Graphics/Vulkan/Graph/FrameGraphBuilder.hpp"

using namespace Droplet::Graphics::Vulkan;

GraphExecutor::GraphExecutor(const Context& context, RenderDevice& device)
    : m_transientPool(context)
{
    // a released target's state must not be carried into a new target that gets the same key
    device.OnResourcesReleased([this, &device](const QueueTimelineValues&)
    {
        ForgetImports([&device](std::uint64_t externalKey)
        {
            return externalKey != SWAPCHAIN_KEY && !device.Resources().Find(TargetFromKey(externalKey));
        });
    });
}

RenderGraph::ResourceState GraphExecutor::ImportState(std::uint64_t externalKey) const
{
    const auto stateIt = m_importStates.find(externalKey);
    return stateIt != m_importStates.end() ? stateIt->second : RenderGraph::ResourceState{};
}

void GraphExecutor::ForgetImports(const std::function<bool(std::uint64_t externalKey)>& isGone)
{
    std::erase_if(m_importStates, [&isGone](const auto& entry) { return isGone(entry.first); });
}

void GraphExecutor::Execute(
    const RenderGraph::Graph<PassContext>& graph,
    const RenderGraph::Compiled& compiled,
    const GraphExecutionInfo& executionInfo,
    const ResolveImportFn& resolveImport)
{
    ZoneScoped
    if (!compiled.ok)
        throw std::logic_error("GraphExecutor::Execute called with a graph that failed to compile");
    if (executionInfo.submitter == nullptr)
        throw std::logic_error("GraphExecutor::Execute needs the frame's FrameSubmitter");
    FrameSubmitter& submitter = *executionInfo.submitter;

    const RenderGraph::GraphData& graphData = graph.Data();
    const auto& resources = graphData.resources;

    // Physical images and buffers for every resource a live pass uses: imports from the caller first (InputRelative
    // transients may be sized from them), then transients from the pool. The frame slot's previous use is finished (see @pre),
    // which the pool relies on.
    ResourceTable resourceTable;
    for (std::uint32_t resourceIndex = 0; resourceIndex < resources.size(); ++resourceIndex)
    {
        if (compiled.lifetimes[resourceIndex].IsUsed() && !resources[resourceIndex].IsTransient())
            resourceTable.Set(RenderGraph::ResourceId{resourceIndex}, resolveImport(resources[resourceIndex]));
    }

    // Reserve this frame's timeline values. The pool tags images with the frame's retire values and frees idle ones
    // once every queue's timeline has passed them.
    submitter.Prepare(compiled, resourceTable, executionInfo.external);
    m_transientPool.BeginFrame(submitter.FrameRetireValues(), submitter.Completed());

    // which submission each step belongs to
    std::vector<std::uint32_t> submissionOfStep(compiled.steps.size(), 0);
    for (std::uint32_t submissionIndex = 0; submissionIndex < compiled.submissions.size(); ++submissionIndex)
    {
        const RenderGraph::Submission& submission = compiled.submissions[submissionIndex];
        for (std::uint32_t stepOffset = 0; stepOffset < submission.stepCount; ++stepOffset)
            submissionOfStep[submission.firstStep + stepOffset] = submissionIndex;
    }

    const auto importedExtent = [&](RenderGraph::ResourceId resource)
    {
        const vk::Extent2D extent = resourceTable.Contains(resource) ? resourceTable.Extent(resource) : vk::Extent2D{};
        return RenderGraph::Extent2D{extent.width, extent.height};
    };
    const RenderGraph::Extent2D swapchainExtent{
        executionInfo.swapchainExtent.width,
        executionInfo.swapchainExtent.height
    };

    std::vector<std::pair<std::uint32_t, PooledImage*>> acquiredTransients;
    std::vector<std::pair<std::uint32_t, PooledBuffer*>> acquiredBuffers;
    for (std::uint32_t resourceIndex = 0; resourceIndex < resources.size(); ++resourceIndex)
    {
        if (!compiled.lifetimes[resourceIndex].IsUsed() || !resources[resourceIndex].IsTransient())
            continue;

        const RenderGraph::ResourceId resourceId{resourceIndex};
        const std::uint32_t firstStep = compiled.lifetimes[resourceIndex].firstStep;

        // a transient buffer: a pooled buffer, ordered after its previous use like a pooled image (below)
        if (resources[resourceIndex].IsBuffer())
        {
            PooledBuffer& pooled = m_transientPool.AcquireBuffer({
                .size = resources[resourceIndex].transientBuffer->size,
                .usage = BufferUsageFromMask(compiled.usageMasks[resourceIndex]),
                .sharedAcrossQueues = std::popcount(compiled.queueMasks[resourceIndex]) > 1
            });

            const bool previousUseOnOtherQueue =
                pooled.lastUse.value != 0 && pooled.lastUse.queue != compiled.steps[firstStep].queue;
            if (previousUseOnOtherQueue)
                submitter.AddCarryWait(submissionOfStep[firstStep],
                                       {.queue = pooled.lastUse.queue, .value = pooled.lastUse.value});

            resourceTable.Set(resourceId, {
                                  .buffer = {
                                      .buffer = pooled.buffer.Get(),
                                      .offset = 0,
                                      .size = pooled.desc.size,
                                      .previousUse = BufferState{
                                          .accessMask = pooled.lastUse.accessMask,
                                          .stageMask = pooled.lastUse.stageMask
                                      },
                                      .previousUseOnOtherQueue = previousUseOnOtherQueue
                                  }
                              });
            acquiredBuffers.emplace_back(resourceIndex, &pooled);
            continue;
        }
        const RenderGraph::TransientDesc& transient = *resources[resourceIndex].transient;
        const RenderGraph::Extent2D extent =
            RenderGraph::ResolveExtent(graphData, resourceId, swapchainExtent, importedExtent);

        PooledImage& pooled = m_transientPool.Acquire(
            {
                .format = static_cast<vk::Format>(transient.format),
                .extent =
                {
                    .width = extent.width,
                    .height = extent.height
                },
                .mipLevels = transient.mipLevels,
                .arrayLayers = transient.layers,
                .samples = static_cast<vk::SampleCountFlagBits>(transient.samples),
                .usage = ImageUsageFromMask(compiled.usageMasks[resourceIndex]),
                .sharedAcrossQueues = std::popcount(compiled.queueMasks[resourceIndex]) > 1
            });

        // The first barrier waits for the image's previous use (an earlier frame's) through its source stages, which
        // only orders work on the queue that use was on. From another queue, the first submission also waits for
        // that queue's timeline.
        const bool previousUseOnOtherQueue =
            pooled.lastUseValue != 0 && pooled.lastUseQueue != compiled.steps[firstStep].queue;
        if (previousUseOnOtherQueue)
            submitter.AddCarryWait(submissionOfStep[firstStep],
                                   {.queue = pooled.lastUseQueue, .value = pooled.lastUseValue});

        resourceTable.Set(resourceId, {
                              .image = {
                                  .image = pooled.image.Get(),
                                  .range = pooled.image.GetSubresourceRange(
                                      Vulkan::Utils::ImageAspectsForFormat(pooled.desc.format)),
                                  .previousUse = pooled.lastUse,
                                  .previousUseOnOtherQueue = previousUseOnOtherQueue
                              },
                              .view = pooled.view.Get(),
                              .extent = pooled.desc.extent
                          });
        acquiredTransients.emplace_back(resourceIndex, &pooled);
    }

    // one command buffer and one submit per graph submission
    graph.Execute(compiled, submitter);

    // Carry each import's final state, with the absolute timeline value of its last use, into next frame's import.
    // Not for imports synchronised from outside (the swapchain image): they are a fresh image every frame.
    for (std::size_t resourceIndex = 0; resourceIndex < resources.size(); ++resourceIndex)
    {
        const RenderGraph::ResourceInfo& info = resources[resourceIndex];
        if (compiled.lastUse[resourceIndex].touched && !info.IsTransient() && !info.options.waitExternal)
            m_importStates[info.externalKey] = FinalState(
                submitter,
                graphData,
                compiled,
                RenderGraph::ResourceId{static_cast<std::uint32_t>(resourceIndex)}
            );
    }

    // Remember how each pooled image was left, on which queue and at which timeline value; the next frame's first
    // use of it waits for this use.
    for (const auto& [resourceIndex, pooled] : acquiredTransients)
    {
        const RenderGraph::Submission& lastSubmission = compiled.submissions[compiled.lastUse[resourceIndex].
            submission];
        pooled->lastUse = ToImageState(compiled.finalUsage[resourceIndex], lastSubmission.queue);
        pooled->lastUseQueue = lastSubmission.queue;
        pooled->lastUseValue = submitter.Absolute(lastSubmission.queue, lastSubmission.signalIndex);
    }
    for (const auto& [resourceIndex, pooled] : acquiredBuffers)
    {
        const RenderGraph::Submission& lastSubmission = compiled.submissions[compiled.lastUse[resourceIndex].
            submission];
        const BufferState state = ToBufferState(compiled.finalUsage[resourceIndex], lastSubmission.queue);
        pooled->lastUse = {
            .stageMask = state.stageMask,
            .accessMask = state.accessMask,
            .queue = lastSubmission.queue,
            .value = submitter.Absolute(lastSubmission.queue, lastSubmission.signalIndex)
        };
    }

    // the slot's next BeginFrame waits for what this frame submitted
    submitter.EndFrame();
}
