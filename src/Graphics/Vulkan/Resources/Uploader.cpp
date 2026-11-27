#include "Graphics/Vulkan/Resources/Uploader.hpp"

#include "Graphics/Vulkan/Core/Context.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "Graphics/Vulkan/Core/VulkanUtils.hpp"

namespace Droplet::Graphics::Vulkan
{
    Uploader::Uploader(const Context& context)
        : m_context(&context),
          m_pool(context.GetDevice(), vk::CommandPoolCreateInfo{
                     .flags = vk::CommandPoolCreateFlagBits::eTransient,
                     .queueFamilyIndex = context.GetGraphicsPresentQueueFamily()
                 }),
          m_fence(context.GetDevice(), vk::FenceCreateInfo{})
    {
    }

    template <class Record>
    void Uploader::SubmitAndWait(const char* name, Record&& record)
    {
        const vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = *m_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };
        vk::raii::CommandBuffers buffers{m_context->GetDevice(), allocInfo};
        vk::raii::CommandBuffer& cmd = buffers.front();

        cmd.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        {
            const GpuZone zone{m_context->GetGpuProfiler(), QueueKind::Graphics, *cmd, name};
            record(*cmd);
        }
        cmd.end();

        const vk::CommandBuffer raw = *cmd;
        const vk::SubmitInfo submit{.commandBufferCount = 1, .pCommandBuffers = &raw};
        m_context->GetGraphicsQueue().submit(submit, *m_fence);

        Utils::CheckResult(
            m_context->GetDevice().waitForFences(*m_fence, vk::True, std::numeric_limits<std::uint64_t>::max()),
            "Upload: waiting for fence failed");
        m_context->GetDevice().resetFences(*m_fence);
    }

    Buffer Uploader::CreateStaging(const void* data, vk::DeviceSize size) const
    {
        if (!data || size == 0)
            throw std::invalid_argument("Data was nullptr or size was 0");

        // Uses the vk::BufferCreateInfo overload (BufferCreateOptions overload currently allocates twice).
        Buffer staging = Buffer::Create(
            m_context->GetAllocator(),
            vk::BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .sharingMode = vk::SharingMode::eExclusive
            },
            vma::AllocationCreateInfo{
                .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                .usage = vma::MemoryUsage::eAuto
            });
        staging.Write(data, size);
        return staging;
    }

    vk::DeviceSize Uploader::BytesPerTexel(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eR8G8B8A8Srgb:
        case vk::Format::eR8G8B8A8Unorm:
            return 4;
            
        case vk::Format::eR32G32B32A32Sfloat:
            return 16;
            
        default:
            throw std::invalid_argument("Unsupported image upload format.");
        }
    }

    Buffer Uploader::CreateDeviceBuffer(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage)
    {
        ZoneScoped
        Buffer staging = CreateStaging(data, size);
        Buffer buffer = Buffer::Create(
            m_context->GetAllocator(),
            vk::BufferCreateInfo{
                .size = size, .usage = usage | vk::BufferUsageFlagBits::eTransferDst,
                .sharingMode = vk::SharingMode::eExclusive
            },
            vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eAutoPreferDevice});

        // fill in lambda
        SubmitAndWait("Upload buffer", [&](vk::CommandBuffer cmd)
        {
            cmd.copyBuffer(staging.Get(), buffer.Get(), vk::BufferCopy{
                               .srcOffset = 0,
                               .dstOffset = 0,
                               .size = size
                           });
        });
        return buffer;
    }

    Image Uploader::UploadImage2D(
        const void* pixels,
        vk::DeviceSize byteSize,
        vk::Extent2D extent,
        vk::Format format,
        std::uint32_t mipLevels)
    {
        ZoneScoped
        
        const vk::DeviceSize bytesPerTexel = BytesPerTexel(format);
        
        if (!pixels || extent.width == 0 || extent.height == 0)
            throw std::invalid_argument("Invalid image upload.");
        
        const vk::DeviceSize expectedBytes = vk::DeviceSize{extent.width} * extent.height * bytesPerTexel;
        if (byteSize != expectedBytes)
            throw std::invalid_argument("Image payload size does not match dimensions and format.");

        // building mips blits each level from the one above with linear filtering, which the format must support
        constexpr vk::FormatFeatureFlags blitFeatures = vk::FormatFeatureFlagBits::eBlitSrc
            | vk::FormatFeatureFlagBits::eBlitDst | vk::FormatFeatureFlagBits::eSampledImageFilterLinear;
        const vk::FormatFeatureFlags formatFeatures = m_context->GetPhysicalDevice().getFormatProperties(format).optimalTilingFeatures;
        if ((formatFeatures & blitFeatures) != blitFeatures)
            mipLevels = 1;
        mipLevels = std::max(mipLevels, 1u);

        Buffer staging = CreateStaging(pixels, byteSize);
        Image image = Image::Create2D(
            m_context->GetAllocator(),
            Image2DCreateInfo{
                .extent = extent, .format = format,
                .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
                         | (mipLevels > 1 ? vk::ImageUsageFlagBits::eTransferSrc : vk::ImageUsageFlags{}),
                .mipLevels = mipLevels
            });

        SubmitAndWait("Upload image", [&](vk::CommandBuffer cmd)
        {
            // one barrier on a range of mip levels
            const auto transitionLevels = [&](std::uint32_t firstLevel, std::uint32_t levelCount,
                                              const ImageState& from, const ImageState& to)
            {
                ImageState state = from;
                TransitionImage(cmd, image.Get(), state, to, vk::ImageSubresourceRange{
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = firstLevel,
                    .levelCount = levelCount,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                });
            };

            transitionLevels(0, mipLevels, ImageState::Undefined(), ImageState::TransferDst());

            const vk::BufferImageCopy region{
                .bufferOffset = 0,
                .bufferRowLength = 0, // tightly packed
                .bufferImageHeight = 0,
                .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                .imageOffset = {0, 0, 0},
                .imageExtent = {extent.width, extent.height, 1},
            };
            cmd.copyBufferToImage(staging.Get(), image.Get(), vk::ImageLayout::eTransferDstOptimal, region);

            // each level from the one above, at half the size (at least 1)
            std::int32_t levelWidth = static_cast<std::int32_t>(extent.width);
            std::int32_t levelHeight = static_cast<std::int32_t>(extent.height);
            for (std::uint32_t level = 1; level < mipLevels; ++level)
            {
                transitionLevels(level - 1, 1, ImageState::TransferDst(), ImageState::TransferSrc());

                const std::int32_t nextWidth = std::max(levelWidth / 2, 1);
                const std::int32_t nextHeight = std::max(levelHeight / 2, 1);
                const vk::ImageBlit blit{
                    .srcSubresource = {vk::ImageAspectFlagBits::eColor, level - 1, 0, 1},
                    .srcOffsets = std::array{vk::Offset3D{0, 0, 0}, vk::Offset3D{levelWidth, levelHeight, 1}},
                    .dstSubresource = {vk::ImageAspectFlagBits::eColor, level, 0, 1},
                    .dstOffsets = std::array{vk::Offset3D{0, 0, 0}, vk::Offset3D{nextWidth, nextHeight, 1}},
                };
                cmd.blitImage(image.Get(), vk::ImageLayout::eTransferSrcOptimal,
                              image.Get(), vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
                levelWidth = nextWidth;
                levelHeight = nextHeight;
            }

            // every level ready for sampling: all but the last were blit sources
            const ImageState sampled = ImageState::Sampled(vk::PipelineStageFlagBits2::eFragmentShader);
            if (mipLevels > 1)
                transitionLevels(0, mipLevels - 1, ImageState::TransferSrc(), sampled);
            transitionLevels(mipLevels - 1, 1, ImageState::TransferDst(), sampled);
        });

        return image;
    }

    void Uploader::ClearToSampled(vk::Image image, const vk::ImageSubresourceRange& range)
    {
        ZoneScoped
        ImageState state = ImageState::Undefined();
        SubmitAndWait("Clear image", [&](vk::CommandBuffer cmd)
        {
            TransitionImage(cmd, image, state, ImageState::TransferDst(), range);
            cmd.clearColorImage(image, vk::ImageLayout::eTransferDstOptimal,
                                vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 1.0f}), range);
            TransitionImage(cmd, image, state, ImageState::Sampled(vk::PipelineStageFlagBits2::eFragmentShader), range);
        });
    }
}
