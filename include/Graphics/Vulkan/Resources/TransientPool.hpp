/**
 * @file TransientPool.hpp
 * @brief Physical images for render-graph transient storage, reused across frames.
 * 
 * @details The render graph relies on transient data, data that is a) only used by it and b) changes every frame.
 * The transient pool stores the images and deletes any that have not been requested in some amount of time.
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Core/Timeline.hpp"
#include "Graphics/RenderGraph/RenderGraphTypes.hpp"
#include "Graphics/Vulkan/Abstraction/Buffer.hpp"
#include "Graphics/Vulkan/Abstraction/Image.hpp"
#include "Graphics/Vulkan/Core/VulkanUtils.hpp"
#include "Graphics/Vulkan/Core/ImageState.hpp"
#include "Graphics/Vulkan/Core/QueueTimelineValues.hpp"

namespace Droplet::Graphics::Vulkan
{
    class Context;

    
    struct TransientImageDesc
    {
        vk::Format format{vk::Format::eUndefined};
        vk::Extent2D extent{};
        std::uint32_t mipLevels{1};
        std::uint32_t arrayLayers{1};
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
        vk::ImageUsageFlags usage{};
        bool sharedAcrossQueues{false}; ///< used on more than one queue: created with concurrent sharing

        friend bool operator==(const TransientImageDesc& lhs, const TransientImageDesc& rhs)
        {
            return lhs.format == rhs.format && lhs.extent.width == rhs.extent.width
                && lhs.extent.height == rhs.extent.height && lhs.mipLevels == rhs.mipLevels
                && lhs.arrayLayers == rhs.arrayLayers && lhs.samples == rhs.samples && lhs.usage == rhs.usage
                && lhs.sharedAcrossQueues == rhs.sharedAcrossQueues;
        }
    };

    struct TransientBufferDesc
    {
        vk::DeviceSize size{0};
        vk::BufferUsageFlags usage{};
        bool sharedAcrossQueues{false}; ///< used on more than one queue: created with concurrent sharing

        friend bool operator==(const TransientBufferDesc&, const TransientBufferDesc&) = default;
    };

    /// @brief Where a pooled resource was last used: what its next first use has to wait for.
    struct PooledLastUse
    {
        vk::PipelineStageFlags2 stageMask{};
        vk::AccessFlags2 accessMask{};
        RenderGraph::Queue queue{RenderGraph::Queue::Graphics};
        TimelineValue value{0}; ///< absolute value that use signalled on `queue`; 0: never used
    };

    /// A single buffer in the pool
    struct PooledBuffer
    {
        TransientBufferDesc desc;
        Buffer buffer;
        PooledLastUse lastUse{};
        std::uint64_t lastUsedFrame{0};             ///< Pool frame (BeginFrame count) of its last Acquire.
        QueueTimelineValues lastUsedRetireValues{}; ///< Retire values of the frame of its last Acquire.
        bool acquiredThisFrame{false};
    };

    /// A single image in the pool and what information the pool stores about it
    struct PooledImage
    {
        TransientImageDesc desc;
        Image image;
        ImageView view;
        // how last frame used this image left it
        ImageState lastUse = ImageState::Undefined();
        RenderGraph::Queue lastUseQueue{RenderGraph::Queue::Graphics}; ///< Queue of that use.
        TimelineValue lastUseValue{0}; ///< Absolute value that use signalled on lastUseQueue; 0: never used.
        std::uint64_t lastUsedFrame{0}; ///< Pool frame (BeginFrame count) of its last Acquire.
        QueueTimelineValues lastUsedRetireValues{}; ///< Retire values of the frame of its last Acquire.
        bool acquiredThisFrame{false};
    };

    /**
     * @brief Hands out pooled images for transients
     * 
     */
    class TransientPool
    {
    public:
        using ImageCreator = std::function<void(PooledImage& pooled)>;
        using BufferCreator = std::function<void(PooledBuffer& pooled)>;

        /**
         * @brief A pool that creates Vulkan images.
         * @param context                  device and allocator; must outlive the pool
         * @param idleFramesBeforeRelease  frames an image may go unrequested before it is destroyed
         */
        explicit TransientPool(const Context& context, std::uint32_t idleFramesBeforeRelease = 3);

        /// @brief A pool with custom creators (for tests).
        TransientPool(ImageCreator imageCreator, BufferCreator bufferCreator, std::uint32_t idleFramesBeforeRelease = 3);

        /**
         * @brief Starts a frame: every image becomes available again, and idle images the GPU is done with are destroyed.
         * @param frameRetireValues timeline values the frame being recorded reaches (FrameSubmitter::FrameRetireValues)
         * @param completedValues   timeline values the GPU has finished (FrameSubmitter::Completed)
         * @details An image is destroyed only if no frame has asked for it in `idleFramesBeforeRelease` frames AND the
         *          last frame that used it has completed on every queue, so it is never destroyed while in use.
         */
        void BeginFrame(const QueueTimelineValues& frameRetireValues, const QueueTimelineValues& completedValues);

        PooledImage& Acquire(const TransientImageDesc& desc);

        /// @brief A buffer matching `desc` that no other transient has this frame.
        PooledBuffer& AcquireBuffer(const TransientBufferDesc& desc);

        [[nodiscard]] std::size_t ImageCount() const noexcept { return m_images.size(); }
        [[nodiscard]] std::size_t BufferCount() const noexcept { return m_buffers.size(); }

    private:
        static void CreateImage(const Context& context, PooledImage& pooled);
        static void CreateBuffer(const Context& context, PooledBuffer& pooled);

        ImageCreator m_creator;
        BufferCreator m_bufferCreator;
        std::uint32_t m_idleFramesBeforeRelease;
        std::uint64_t m_frameIndex{0};              ///< BeginFrame count
        QueueTimelineValues m_frameRetireValues{};  ///< of the frame being recorded
        std::vector<std::unique_ptr<PooledImage>> m_images;
        std::vector<std::unique_ptr<PooledBuffer>> m_buffers;
    };
}
