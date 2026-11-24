/**
 * @file FrameSubmitter.cpp
 * @brief Submission of a frame
 */
#include "Integration/Vulkan/Frame/FrameSubmitter.hpp"

#include <stdexcept>
#include <utility>

using namespace Vulkan;

namespace
{
    using QueueSlots = std::array<FrameSubmitter::QueueSlot, RenderGraph::QUEUE_COUNT>;
    using SlotCommandPools = std::array<vk::raii::CommandPool, RenderGraph::QUEUE_COUNT>;


    /// helper for creating timeline semaphores RAII
    template <std::size_t... QueueIndex>
    std::array<Semaphore, sizeof...(QueueIndex)> MakeTimelines(
        const vk::raii::Device& device,
        std::index_sequence<QueueIndex...>)
    {
        // one timeline per logical queue, whether or not it has a physical queue of its own
        return {((void)QueueIndex, Semaphore(device, Semaphore::Type::Timeline))...};
    }


    /// @brief queues with every logical queue that has no physical queue of its own sent to the graphics queue
    /// @throws std::invalid_argument if there is no graphics queue
    QueueSlots WithGraphicsFallback(QueueSlots queues)
    {
        constexpr std::size_t graphicsIndex = RenderGraph::ToIndex(RenderGraph::Queue::Graphics);
        if (!queues[graphicsIndex].queue)
            throw std::invalid_argument("FrameSubmitter needs a graphics queue");

        // a logical queue without a physical one records and submits on the graphics queue
        for (FrameSubmitter::QueueSlot& slot : queues)
        {
            if (!slot.queue)
                slot = queues[graphicsIndex];
        }
        return queues;
    }

    /// @brief One frame slot's command pools: one per logical queue, for that queue's family
    template <std::size_t... QueueIndex>
    SlotCommandPools MakeSlotCommandPools(
        const vk::raii::Device& device, const QueueSlots& queues, std::index_sequence<QueueIndex...>)
    {
        // transient: the buffers live for one frame; the pool is reset when the slot is reused
        return {
            vk::raii::CommandPool(device, vk::CommandPoolCreateInfo{
                                      .flags = vk::CommandPoolCreateFlagBits::eTransient,
                                      .queueFamilyIndex = queues[QueueIndex].family
                                  })...
        };
    }
}

FrameSubmitter::FrameSubmitter(
    const vk::raii::Device& device,
    std::array<QueueSlot, RenderGraph::QUEUE_COUNT> queues,
    std::uint32_t framesInFlight)
    : m_device(&device),
      m_queues(WithGraphicsFallback(std::move(queues))),
      m_timelines(MakeTimelines(device, std::make_index_sequence<RenderGraph::QUEUE_COUNT>{}))
{
    m_slots.reserve(framesInFlight);
    for (std::uint32_t slot = 0; slot < framesInFlight; ++slot)
    {
        m_slots.push_back(
            FrameSlot{
                .commandPools = MakeSlotCommandPools(device, m_queues,
                                                     std::make_index_sequence<RenderGraph::QUEUE_COUNT>{})
            }
        );
    }
}

void FrameSubmitter::BeginFrame(std::uint32_t frameSlot)
{
    m_frameSlot = frameSlot;
    FrameSlot& slot = m_slots[frameSlot];

    // wait until the GPU is past everything the slot's previous frame submitted, on every queue
    for (std::size_t queueIndex = 0; queueIndex < RenderGraph::QUEUE_COUNT; ++queueIndex)
    {
        m_timelines[queueIndex].Wait(slot.retireValues.values[queueIndex]);
    }

    // its command buffers are free now: drop them, then reset the pools they came from
    slot.commandBuffers.clear();
    for (auto& commandPool : slot.commandPools)
    {
        commandPool.reset();
    }
}

void FrameSubmitter::Prepare(const RenderGraph::Compiled& compiled, const ResourceTable& resources, External external)
{
    m_compiled = &compiled;
    m_resources = &resources;
    m_external = std::move(external);
    m_extraCarryWaits.assign(compiled.submissions.size(), {});

    // this frame signals values base + 1 .. base + count on each queue
    for (std::size_t queueIndex = 0; queueIndex < RenderGraph::QUEUE_COUNT; ++queueIndex)
    {
        m_frameBase[queueIndex] = m_timelines[queueIndex].Reserve(compiled.signalsPerQueue[queueIndex]);
        m_frameRetire.values[queueIndex] = m_frameBase[queueIndex] + compiled.signalsPerQueue[queueIndex];
    }
}

void FrameSubmitter::AddCarryWait(std::uint32_t submissionIndex, RenderGraph::CarryWait carryWait)
{
    m_extraCarryWaits.at(submissionIndex).push_back(carryWait);
}

PassContext& FrameSubmitter::Begin(const RenderGraph::Submission& submission)
{
    FrameSlot& slot = m_slots[m_frameSlot];

    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *slot.commandPools[RenderGraph::ToIndex(submission.queue)],
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffers commandBuffers{*m_device, allocateInfo};
    slot.commandBuffers.push_back(std::move(commandBuffers.front()));

    vk::raii::CommandBuffer& commandBuffer = slot.commandBuffers.back();
    commandBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    m_passContext = PassContext{
        .cmd = *commandBuffer,
        .queue = submission.queue,
        .resources = m_resources
    };
    return m_passContext;
}

void FrameSubmitter::Barriers(PassContext& context, std::span<const RenderGraph::Barrier> barriers)
{
    RecordBarriers(context.cmd, context.queue, barriers, *m_resources,
                   m_queues[RenderGraph::ToIndex(context.queue)].canDraw);
}

void FrameSubmitter::End(const PassContext& context, const RenderGraph::Submission& submission)
{
    m_slots[m_frameSlot].commandBuffers.back().end();

    std::vector<vk::SemaphoreSubmitInfo> waits;
    std::vector<vk::SemaphoreSubmitInfo> signals;

    // earlier submissions of this frame on other queues
    for (const std::uint32_t sourceIndex : submission.waits)
    {
        const RenderGraph::Submission& source = m_compiled->submissions[sourceIndex];
        waits.push_back(Timeline(source.queue).SubmitInfo(
            vk::PipelineStageFlagBits2::eAllCommands, Absolute(source.queue, source.signalIndex)));
    }
    // earlier frames on other queues: imports whose last use carried an absolute value, and pooled images whose
    // last use did (AddCarryWait)
    const std::size_t submissionIndex = static_cast<std::size_t>(&submission - m_compiled->submissions.data());
    const auto waitForEarlierFrame = [this, &waits](const RenderGraph::CarryWait& carryWait)
    {
        waits.push_back(
            Timeline(carryWait.queue).SubmitInfo(vk::PipelineStageFlagBits2::eAllCommands, carryWait.value));
    };
    for (const RenderGraph::CarryWait& carryWait : submission.carryWaits)
        waitForEarlierFrame(carryWait);
    for (const RenderGraph::CarryWait& carryWait : m_extraCarryWaits[submissionIndex])
        waitForEarlierFrame(carryWait);
    // external binary semaphores: the swapchain image is written as a colour attachment, and its first barrier
    // starts at that stage
    for (const RenderGraph::ResourceId resource : submission.waitExternal)
        waits.push_back(m_external.acquire(resource).SubmitInfo(vk::PipelineStageFlagBits2::eColorAttachmentOutput));

    signals.push_back(Timeline(submission.queue).SubmitInfo(
        vk::PipelineStageFlagBits2::eAllCommands, Absolute(submission.queue, submission.signalIndex)));
    for (const RenderGraph::ResourceId resource : submission.signalExternal)
        signals.push_back(m_external.finished(resource).SubmitInfo(vk::PipelineStageFlagBits2::eAllCommands));

    const vk::CommandBufferSubmitInfo commandBufferInfo{.commandBuffer = context.cmd};
    const vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount = static_cast<std::uint32_t>(waits.size()),
        .pWaitSemaphoreInfos = waits.data(),
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &commandBufferInfo,
        .signalSemaphoreInfoCount = static_cast<std::uint32_t>(signals.size()),
        .pSignalSemaphoreInfos = signals.data(),
    };
    m_queues[RenderGraph::ToIndex(submission.queue)].queue->submit2(submitInfo);
}

void FrameSubmitter::EndFrame()
{
    m_slots[m_frameSlot].retireValues = m_frameRetire;
    m_compiled = nullptr;
    m_resources = nullptr;
    m_external = {};
    m_extraCarryWaits.clear();
}

TimelineValue FrameSubmitter::Absolute(RenderGraph::Queue queue, std::uint32_t signalIndex) const
{
    return m_frameBase[RenderGraph::ToIndex(queue)] + signalIndex;
}

QueueTimelineValues FrameSubmitter::Completed() const
{
    QueueTimelineValues completed;
    for (std::size_t queueIndex = 0; queueIndex < RenderGraph::QUEUE_COUNT; ++queueIndex)
    {
        completed.values[queueIndex] = m_timelines[queueIndex].Completed();
    }
    return completed;
}

const Semaphore& FrameSubmitter::Timeline(RenderGraph::Queue queue) const
{
    return m_timelines[RenderGraph::ToIndex(queue)];
}

RenderGraph::ResourceState Vulkan::FinalState(
    const FrameSubmitter& submitter,
    const RenderGraph::GraphData& graph,
    const RenderGraph::Compiled& compiled,
    RenderGraph::ResourceId resource)
{
    const RenderGraph::LastUse& lastUse = compiled.lastUse[resource.index];
    if (!lastUse.touched)
    {        
    return graph.resources[resource.index].initialState;
    }

    const RenderGraph::Submission& submission = compiled.submissions[lastUse.submission];
    return {
        .usage = compiled.finalUsage[resource.index],
        .queue = submission.queue,
        .value = submitter.Absolute(submission.queue, submission.signalIndex)
    };
}
