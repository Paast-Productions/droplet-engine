#include "Integration/Vulkan/Resources/TransientPool.hpp"

#include "Integration/Vulkan/Core/Context.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

using namespace Vulkan;

TransientPool::TransientPool(const Context& context, std::uint32_t idleFramesBeforeRelease)
    : TransientPool([contextPointer = &context](PooledImage& pooled) { CreateImage(*contextPointer, pooled); },
        [contextPointer = &context](PooledBuffer& pooled) { CreateBuffer(*contextPointer, pooled); },
        idleFramesBeforeRelease)
{
}

TransientPool::TransientPool(
    ImageCreator imageCreator, BufferCreator bufferCreator, std::uint32_t idleFramesBeforeRelease)
    : m_creator(std::move(imageCreator)),
    m_bufferCreator(std::move(bufferCreator)),
    m_idleFramesBeforeRelease(idleFramesBeforeRelease)
{
}

void TransientPool::BeginFrame(
    const QueueTimelineValues& frameRetireValues, const QueueTimelineValues& completedValues)
{
    ++m_frameIndex;
    m_frameRetireValues = frameRetireValues;

    // release images that are idle AND that the GPU has finished with, on every queue
    std::erase_if(m_images, [&](const std::unique_ptr<PooledImage>& pooled)
        {
            const bool idle = m_frameIndex - pooled->lastUsedFrame >= m_idleFramesBeforeRelease;
            const bool finishedOnGpu = pooled->lastUsedRetireValues.ReachedBy(completedValues);
            return idle && finishedOnGpu;
        });

    std::erase_if(m_buffers, [&](const std::unique_ptr<PooledBuffer>& pooled)
        {
            const bool idle = m_frameIndex - pooled->lastUsedFrame >= m_idleFramesBeforeRelease;
            return idle && pooled->lastUsedRetireValues.ReachedBy(completedValues);
        });

    for (const auto& pooled : m_images)
    {
        pooled->acquiredThisFrame = false;
    }
    for (const auto& pooled : m_buffers)
    {
        pooled->acquiredThisFrame = false;
    }
}

PooledBuffer& TransientPool::AcquireBuffer(const TransientBufferDesc& desc)
{
    for (const std::unique_ptr<PooledBuffer>& pooled : m_buffers)
    {
        if (pooled->acquiredThisFrame || pooled->desc != desc)
            continue;
        pooled->acquiredThisFrame = true;
        pooled->lastUsedFrame = m_frameIndex;
        pooled->lastUsedRetireValues = m_frameRetireValues;
        return *pooled;
    }

    auto created = std::make_unique<PooledBuffer>();
    created->desc = desc;
    m_bufferCreator(*created);
    created->acquiredThisFrame = true;
    created->lastUsedFrame = m_frameIndex;
    created->lastUsedRetireValues = m_frameRetireValues;
    m_buffers.push_back(std::move(created));
    return *m_buffers.back();
}

PooledImage& TransientPool::Acquire(const TransientImageDesc& desc)
{
    for (const std::unique_ptr<PooledImage>& pooled : m_images)
    {
        if (pooled->acquiredThisFrame || (pooled->desc != desc))
            continue;
        pooled->acquiredThisFrame = true;
        pooled->lastUsedFrame = m_frameIndex;
        pooled->lastUsedRetireValues = m_frameRetireValues;
        return *pooled;
    }

    if (desc.arrayLayers != 1)
        throw std::invalid_argument("image arrays are not supported yet");

    auto created = std::make_unique<PooledImage>();
    created->desc = desc;
    m_creator(*created);
    created->acquiredThisFrame = true;
    created->lastUsedFrame = m_frameIndex;
    created->lastUsedRetireValues = m_frameRetireValues;
    m_images.push_back(std::move(created));
    return *m_images.back();
}

void TransientPool::CreateImage(const Context& context, PooledImage& pooled)
{
    pooled.image = Image::Create2D(
        context.GetAllocator(),
        Image2DCreateInfo{
            .extent = pooled.desc.extent,
            .format = pooled.desc.format,
            .usage = pooled.desc.usage,
            .mipLevels = pooled.desc.mipLevels,
            .arrayLayers = pooled.desc.arrayLayers,
            .sharedQueueFamilies = pooled.desc.sharedAcrossQueues
                                       ? context.GetQueueInfo().UniqueFamilies()
                                       : std::vector<std::uint32_t>{}
        });
    pooled.view = ImageView::Create2D(
        context.GetDevice(), pooled.image,
        ImageViewCreateOptions{
            .aspectMask = Utils::ImageAspectsForFormat(pooled.desc.format),
            .levelCount = pooled.desc.mipLevels });
}

void TransientPool::CreateBuffer(const Context& context, PooledBuffer& pooled)
{
    const std::vector<std::uint32_t> families = pooled.desc.sharedAcrossQueues
        ? context.GetQueueInfo().UniqueFamilies()
        : std::vector<std::uint32_t>{};
    const bool concurrent = families.size() > 1;
    pooled.buffer = Buffer::Create(context.GetAllocator(), vk::BufferCreateInfo{
        .size = pooled.desc.size,
        .usage = pooled.desc.usage,
        .sharingMode = concurrent ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = concurrent ? static_cast<std::uint32_t>(families.size()) : 0u,
        .pQueueFamilyIndices = concurrent ? families.data() : nullptr
        });
}
