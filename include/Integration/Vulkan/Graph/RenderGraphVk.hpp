/** 
 * @file RenderGraphVk.hpp
 * @brief Core transition from render graph (no vulkan) to Vulkan-executable stuff.
 * 
 * @details Vulkan binding for the render graph core: graph usages as image states and usage flags, barrier recording,
 * the per-frame ResourceTable and the PassContext pass callbacks receive. Submission (one command buffer and one
 * submit per graph submission, per-queue timelines) is Vulkan::FrameSubmitter (Vulkan/Frame/FrameSubmitter.hpp).
 */

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Core/Timeline.hpp"
#include "Integration/Vulkan/Abstraction/Device.hpp"
#include "Integration/Vulkan/Core/ImageState.hpp"
#include "Integration/Vulkan/Core/VulkanUtils.hpp"
#include "Integration/RenderGraph/RenderGraph.hpp"

namespace Vulkan
{
    static_assert(ToIndex(QueueKind::Graphics) == RenderGraph::ToIndex(RenderGraph::Queue::Graphics));
    static_assert(ToIndex(QueueKind::Compute) == RenderGraph::ToIndex(RenderGraph::Queue::Compute));
    static_assert(ToIndex(QueueKind::Transfer) == RenderGraph::ToIndex(RenderGraph::Queue::Transfer));
    static_assert(QUEUE_INDEX_COUNT == RenderGraph::QUEUE_COUNT);

    /// @brief The device queue a logical graph queue stands for (the two enums have the same order)
    constexpr QueueKind ToQueueIndex(RenderGraph::Queue queue) noexcept
    {
        return static_cast<QueueKind>(RenderGraph::ToIndex(queue));
    }

    /// @brief The physical image behind a graph resource, as a barrier needs it
    struct ResolvedImage
    {
        vk::Image image{};
        vk::ImageSubresourceRange range{};
        /// How the physical image was last used, for an image whose contents this frame discards (a transient's
        /// first access). The first barrier then keeps oldLayout = UNDEFINED but waits on this stage and access, so a
        /// pooled image reused from an earlier frame is ordered after its previous user. Empty when there was none.
        std::optional<ImageState> previousUse{};
        /// previousUse was on another queue than the first access: the submission waits on that queue's timeline,
        /// and the barrier may not name previousUse's stages (see MakeImageBarrier).
        bool previousUseOnOtherQueue{false};
    };

    /// @brief Stages and accesses of one use of a buffer (a buffer has no layout).
    struct BufferState
    {
        vk::AccessFlags2 accessMask{};
        vk::PipelineStageFlags2 stageMask{};
    };

    /// @brief The physical buffer behind a graph resource, as a barrier and a descriptor need it.
    struct ResolvedBuffer
    {
        vk::Buffer buffer{};
        vk::DeviceSize offset{0};
        vk::DeviceSize size{vk::WholeSize};
        /// How the buffer was last used, for a buffer whose contents this frame discards (a transient's first
        /// access): the first barrier waits on it. Empty when there was none.
        std::optional<BufferState> previousUse{};
        bool previousUseOnOtherQueue{false}; ///< see ResolvedImage::previousUseOnOtherQueue
    };

    /// @brief A graph resource's physical image (view, size) or buffer for one frame.
    struct PhysicalResource
    {
        ResolvedImage image{};   ///< What barriers use (images)
        vk::ImageView view{};    ///< What attachments and descriptors use (images)
        vk::Extent2D extent{};   ///< Size in pixels (images)
        ResolvedBuffer buffer{}; ///< Set for a buffer resource; then the image members are empty

        [[nodiscard]] bool IsBuffer() const noexcept { return buffer.buffer != vk::Buffer{}; }
    };

    /**
     * @brief Maps one frame's graph resources to their physical images
     * @details Filled before Graph::Execute with an entry for every resource a live pass uses (imported targets and
     * transient resources). Barriers resolve through it, and pass callbacks reach it through PassContext::resources.
     */
    class ResourceTable
    {
    public:
        /// @brief Records the physical image behind `resource`, replacing any earlier entry.
        void Set(RenderGraph::ResourceId resource, const PhysicalResource& physical)
        {
            if (resource.index >= m_entries.size())
                m_entries.resize(resource.index + 1);
            m_entries[resource.index] = physical;
        }

        /// @brief True if `resource` has an entry.
        [[nodiscard]] bool Contains(RenderGraph::ResourceId resource) const noexcept
        {
            return resource.index < m_entries.size() && m_entries[resource.index].has_value();
        }

        /// @brief The entry for `resource`.
        /// @throws std::out_of_range if it has none (the resource is not used by a live pass, or was never set)
        [[nodiscard]] const PhysicalResource& Get(RenderGraph::ResourceId resource) const
        {
            if (!Contains(resource))
                throw std::out_of_range("ResourceTable: no physical image for resource " + std::to_string(resource.index));
            return *m_entries[resource.index];
        }

        [[nodiscard]] ResolvedImage Resolve(RenderGraph::ResourceId resource) const { return Get(resource).image; }
        [[nodiscard]] const ResolvedBuffer& Buffer(RenderGraph::ResourceId resource) const { return Get(resource).buffer; }
        [[nodiscard]] vk::ImageView View(RenderGraph::ResourceId resource) const { return Get(resource).view; }
        [[nodiscard]] vk::Extent2D Extent(RenderGraph::ResourceId resource) const { return Get(resource).extent; }

    private:
        std::vector<std::optional<PhysicalResource>> m_entries;
    };

    ///@brief  What every pass callback receives. Passes begin/end dynamic rendering themselves
    struct PassContext
    {
        vk::CommandBuffer cmd;
        RenderGraph::Queue queue = RenderGraph::Queue::Graphics; ///< Queue this pass is running on
        const ResourceTable* resources = nullptr; ///< This frame's physical images; set by the recorder.
    };

    /// @brief deduce image state from usage and submit queue
    /// @details Stage masks must exist on the queue that records the barrier, so sampling is
    /// a fragment-shader read on graphics and a compute-shader read on compute
    inline ImageState ToImageState(RenderGraph::Usage usage, RenderGraph::Queue queue)
    {
        using U = RenderGraph::Usage;
        switch (usage)
        {
        case U::None: return ImageState::Undefined();
        case U::ColorWrite: return ImageState::ColorAttachment();
        case U::DepthWrite: return ImageState::DepthAttachment();
        case U::SampledRead:
            return ImageState::Sampled(queue == RenderGraph::Queue::Compute
                                           ? vk::PipelineStageFlagBits2::eComputeShader
                                           : vk::PipelineStageFlagBits2::eFragmentShader);
        case U::StorageWrite:
            return ImageState{
                .layout = vk::ImageLayout::eGeneral,
                .accessMask = vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite,
                .stageMask = vk::PipelineStageFlagBits2::eComputeShader
            };
        case U::StorageRead:
            return ImageState{
                .layout = vk::ImageLayout::eGeneral,
                .accessMask = vk::AccessFlagBits2::eShaderStorageRead,
                .stageMask = queue == RenderGraph::Queue::Compute
                                 ? vk::PipelineStageFlags2{vk::PipelineStageFlagBits2::eComputeShader}
                                 : vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader
            };
        case U::TransferSrc: return ImageState::TransferSrc();
        case U::TransferDst: return ImageState::TransferDst();
        case U::Present: return ImageState::Present();
        default: break; // buffer-only usages; the graph rejects them on images
        }
        return ImageState::Undefined();
    }

    /// @brief The stages and accesses of a buffer used as `usage` on `queue`.
    /// @details Stage masks must exist on the queue that records the barrier: a shader access is a compute-shader
    ///          access on the compute queue, and a vertex, fragment or compute-shader access on the graphics queue.
    inline BufferState ToBufferState(RenderGraph::Usage usage, RenderGraph::Queue queue)
    {
        using U = RenderGraph::Usage;
        const vk::PipelineStageFlags2 shaderStages = queue == RenderGraph::Queue::Compute
                                                         ? vk::PipelineStageFlags2{vk::PipelineStageFlagBits2::eComputeShader}
                                                         : vk::PipelineStageFlagBits2::eVertexShader
                                                         | vk::PipelineStageFlagBits2::eFragmentShader
                                                         | vk::PipelineStageFlagBits2::eComputeShader;
        switch (usage)
        {
        case U::StorageRead: return {vk::AccessFlagBits2::eShaderStorageRead, shaderStages};
        case U::StorageWrite:
            return {vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite, shaderStages};
        case U::UniformRead: return {vk::AccessFlagBits2::eUniformRead, shaderStages};
        case U::IndirectRead:
            return {vk::AccessFlagBits2::eIndirectCommandRead, vk::PipelineStageFlagBits2::eDrawIndirect};
        case U::VertexRead:
            return {vk::AccessFlagBits2::eVertexAttributeRead, vk::PipelineStageFlagBits2::eVertexAttributeInput};
        case U::IndexRead: return {vk::AccessFlagBits2::eIndexRead, vk::PipelineStageFlagBits2::eIndexInput};
        case U::TransferSrc: return {vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eTransfer};
        case U::TransferDst: return {vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTransfer};
        default: return {};
        }
    }

    /// @brief Vulkan buffer usage flags for every usage in a usage mask (Compiled::usageMasks).
    inline vk::BufferUsageFlags BufferUsageFromMask(std::uint32_t usageMask)
    {
        using RenderGraph::Usage;
        using RenderGraph::UsageBit;
        vk::BufferUsageFlags usage{};
        if (usageMask & (UsageBit(Usage::StorageRead) | UsageBit(Usage::StorageWrite)))
            usage |= vk::BufferUsageFlagBits::eStorageBuffer;
        if (usageMask & UsageBit(Usage::UniformRead))  usage |= vk::BufferUsageFlagBits::eUniformBuffer;
        if (usageMask & UsageBit(Usage::IndirectRead)) usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
        if (usageMask & UsageBit(Usage::VertexRead))   usage |= vk::BufferUsageFlagBits::eVertexBuffer;
        if (usageMask & UsageBit(Usage::IndexRead))    usage |= vk::BufferUsageFlagBits::eIndexBuffer;
        if (usageMask & UsageBit(Usage::TransferSrc))  usage |= vk::BufferUsageFlagBits::eTransferSrc;
        if (usageMask & UsageBit(Usage::TransferDst))  usage |= vk::BufferUsageFlagBits::eTransferDst;
        return usage;
    }

    /**
     * @brief Vulkan image usage flags for every usage in a usage mask (Compiled::usageMasks).
     * @param usageMask bits made with RenderGraph::UsageBit
     */
    inline vk::ImageUsageFlags ImageUsageFromMask(std::uint32_t usageMask)
    {
        using RenderGraph::Usage;
        using RenderGraph::UsageBit;
        vk::ImageUsageFlags usage{};
        if (usageMask & UsageBit(Usage::ColorWrite))   usage |= vk::ImageUsageFlagBits::eColorAttachment;
        if (usageMask & UsageBit(Usage::DepthWrite))   usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        if (usageMask & UsageBit(Usage::SampledRead))  usage |= vk::ImageUsageFlagBits::eSampled;
        if (usageMask & (UsageBit(Usage::StorageRead) | UsageBit(Usage::StorageWrite)))
            usage |= vk::ImageUsageFlagBits::eStorage;
        if (usageMask & UsageBit(Usage::TransferSrc))  usage |= vk::ImageUsageFlagBits::eTransferSrc;
        if (usageMask & UsageBit(Usage::TransferDst))  usage |= vk::ImageUsageFlagBits::eTransferDst;
        return usage;
    }

    /// @brief Builds the image barrier for one graph barrier recorded on `queue`.
    /// @details After an access on another queue (ordered by the submission's timeline wait), the source stage is
    ///          ALL_COMMANDS: the other access's stage may not exist on this queue's family. A first access of a pooled
    ///          image keeps oldLayout UNDEFINED but waits on its previous use (ResolvedImage::previousUse).
    inline vk::ImageMemoryBarrier2 MakeImageBarrier(
        const RenderGraph::Barrier& barrier,
        RenderGraph::Queue queue,
        const ResolvedImage& resolved,
        bool recordingFamilyCanDraw)
    {
        bool fromOtherQueue = barrier.fromQueue != queue;
        ImageState fromState = ToImageState(barrier.from, barrier.fromQueue);
        const ImageState toState = ToImageState(barrier.to, queue);

        if (barrier.from == RenderGraph::Usage::None && resolved.previousUse)
        {
            fromState.stageMask = resolved.previousUse->stageMask;
            fromState.accessMask = resolved.previousUse->accessMask;
            fromOtherQueue = resolved.previousUseOnOtherQueue;
            // fromState.layout stays UNDEFINED: this frame does not need the old contents.
        }

        return {
            .srcStageMask = fromOtherQueue ? vk::PipelineStageFlags2{vk::PipelineStageFlagBits2::eAllCommands}
                                           : fromState.stageMask,
            // after another queue, the semaphore wait already made that queue's writes available and visible; the
            // access is only named where it is valid: on a family that can draw, ALL_COMMANDS covers every access,
            // on a compute-only or transfer-only family it does not (VUID-VkImageMemoryBarrier2-srcAccessMask-03910)
            .srcAccessMask = fromOtherQueue && !recordingFamilyCanDraw ? vk::AccessFlags2{} : fromState.accessMask,
            .dstStageMask = toState.stageMask,
            .dstAccessMask = toState.accessMask,
            .oldLayout = fromState.layout,
            .newLayout = toState.layout,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image = resolved.image,
            .subresourceRange = resolved.range,
        };
    }

    /// @brief Builds the buffer barrier for one graph barrier recorded on `queue`; same rules as MakeImageBarrier.
    inline vk::BufferMemoryBarrier2 MakeBufferBarrier(
        const RenderGraph::Barrier& barrier,
        RenderGraph::Queue queue,
        const ResolvedBuffer& resolved,
        bool recordingFamilyCanDraw)
    {
        bool fromOtherQueue = barrier.fromQueue != queue;
        BufferState fromState = ToBufferState(barrier.from, barrier.fromQueue);
        const BufferState toState = ToBufferState(barrier.to, queue);

        if (barrier.from == RenderGraph::Usage::None && resolved.previousUse)
        {
            fromState = *resolved.previousUse;
            fromOtherQueue = resolved.previousUseOnOtherQueue;
        }

        return {
            .srcStageMask = fromOtherQueue ? vk::PipelineStageFlags2{vk::PipelineStageFlagBits2::eAllCommands}
                                           : fromState.stageMask,
            // after another queue, the semaphore wait already made that queue's writes available and visible; the
            // access is only named where it is valid: on a family that can draw, ALL_COMMANDS covers every access,
            // on a compute-only or transfer-only family it does not (VUID-VkImageMemoryBarrier2-srcAccessMask-03910)
            .srcAccessMask = fromOtherQueue && !recordingFamilyCanDraw ? vk::AccessFlags2{} : fromState.accessMask,
            .dstStageMask = toState.stageMask,
            .dstAccessMask = toState.accessMask,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .buffer = resolved.buffer,
            .offset = resolved.offset,
            .size = resolved.size,
        };
    }

    /// One pipelineBarrier2 for a batch, recorded on `queue` (see MakeImageBarrier for accesses on another queue).
    /// Resources are expected to be VK_SHARING_MODE_CONCURRENT when they cross queues, so no ownership transfer.
    inline void RecordBarriers(
        vk::CommandBuffer cmd,
        RenderGraph::Queue queue,
        std::span<const RenderGraph::Barrier> barriers,
        const ResourceTable& resources,
        bool recordingFamilyCanDraw)
    {
        std::vector<vk::ImageMemoryBarrier2> imageBarriers;
        std::vector<vk::BufferMemoryBarrier2> bufferBarriers;
        for (const RenderGraph::Barrier& barrier : barriers)
        {
            const PhysicalResource& physical = resources.Get(barrier.resourceId);
            if (physical.IsBuffer())
                bufferBarriers.push_back(MakeBufferBarrier(barrier, queue, physical.buffer, recordingFamilyCanDraw));
            else
                imageBarriers.push_back(MakeImageBarrier(barrier, queue, physical.image, recordingFamilyCanDraw));
        }

        cmd.pipelineBarrier2(vk::DependencyInfo{
            .bufferMemoryBarrierCount = static_cast<std::uint32_t>(bufferBarriers.size()),
            .pBufferMemoryBarriers = bufferBarriers.data(),
            .imageMemoryBarrierCount = static_cast<std::uint32_t>(imageBarriers.size()),
            .pImageMemoryBarriers = imageBarriers.data()
        });
    }
}
