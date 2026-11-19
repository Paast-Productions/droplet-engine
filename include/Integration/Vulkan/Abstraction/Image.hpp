/**
 * @file Image.hpp
 * @author Sebastian Lorensson
 * @brief Abstraction wrapper around vulkan Image
 * @date 2026-11-02
 */
#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include "Integration/Vulkan/Core/ImageState.hpp"

namespace Vulkan
{
    /// create info struct to avoid passing a lot of parameters when creating an Image
    struct Image2DCreateInfo
    {
        vk::Extent2D extent{};
        vk::Format format = vk::Format::eUndefined;
        vk::ImageUsageFlags usage{};
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
        std::uint32_t mipLevels = 1;
        std::uint32_t arrayLayers = 1;
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::ImageCreateFlags flags{};
        std::vector<std::uint32_t> sharedQueueFamilies{};
        // queue families that use the image without ownership transfers 

        vma::AllocationCreateInfo allocationInfo{
            .usage = vma::MemoryUsage::eAutoPreferDevice
        };
    };

    /// vk::Image convenience wrapper
    class Image
    {
    public:
        Image() = default;
        ~Image() = default;
        
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        Image(Image&& other) noexcept
            : m_image(std::move(other.m_image)),
              m_imageState(std::exchange(other.m_imageState, ImageState::Undefined())),
              m_extent(std::exchange(other.m_extent, vk::Extent3D{})),
              m_format(std::exchange(other.m_format, vk::Format::eUndefined)),
              m_usage(std::exchange(other.m_usage, vk::ImageUsageFlags{})),
              m_imageType(std::exchange(other.m_imageType, vk::ImageType::e1D)),
              m_mipLevels(std::exchange(other.m_mipLevels, 0)),
              m_arrayLayers(std::exchange(other.m_arrayLayers, 0))
        {
        }


        Image& operator=(Image&& other) noexcept
        {
            if (this == &other)
                return *this;

            // destroy currently owned image and allocation
            m_image.clear();
            m_image = std::move(other.m_image);

            m_imageState = std::exchange(other.m_imageState, ImageState::Undefined());
            m_extent = std::exchange(other.m_extent, vk::Extent3D{});
            m_format = std::exchange(other.m_format, vk::Format::eUndefined);
            m_usage = std::exchange(other.m_usage, vk::ImageUsageFlags{});
            m_imageType = std::exchange(other.m_imageType, vk::ImageType::e1D);
            m_mipLevels = std::exchange(other.m_mipLevels, 0);
            m_arrayLayers = std::exchange(other.m_arrayLayers, 0);

            return *this;
        }

        /// Create as image
        static Image Create(
            const vma::raii::Allocator& allocator,
            const vk::ImageCreateInfo& createInfo,
            const vma::AllocationCreateInfo& allocationCreateInfo =
            {
                .usage = vma::MemoryUsage::eAutoPreferDevice
            })
        {
            vma::raii::Image image = allocator.createImage(createInfo, allocationCreateInfo);

            return {
                std::move(image),
                createInfo
            };
        }

        /// Create as 2D image
        static Image Create2D(
            const vma::raii::Allocator& allocator,
            const Image2DCreateInfo& createInfo)
        {
            if (createInfo.extent.width == 0 || createInfo.extent.height == 0 || createInfo.mipLevels == 0 ||
                createInfo.arrayLayers == 0 || createInfo.format == vk::Format::eUndefined || !createInfo.usage)
            {
                throw std::invalid_argument("Invalid image creation parameters.");
            }

            // if more than one queue wants access to this without ownership transfer we need to set to concurrent 
            const bool shared = createInfo.sharedQueueFamilies.size() > 1;

            vk::ImageCreateInfo info{};

            info.flags = createInfo.flags;
            info.imageType = vk::ImageType::e2D;
            info.extent = vk::Extent3D{
                .width = createInfo.extent.width,
                .height = createInfo.extent.height,
                .depth = 1
            };
            info.mipLevels = createInfo.mipLevels;
            info.arrayLayers = createInfo.arrayLayers;
            info.format = createInfo.format;
            info.tiling = createInfo.tiling;
            info.initialLayout = vk::ImageLayout::eUndefined;
            info.usage = createInfo.usage;
            info.samples = createInfo.samples;
            info.sharingMode = shared ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
            info.queueFamilyIndexCount = shared ? static_cast<uint32_t>(createInfo.sharedQueueFamilies.size()) : 0;
            info.pQueueFamilyIndices = shared ? createInfo.sharedQueueFamilies.data() : nullptr;

            return Create(
                allocator,
                info,
                createInfo.allocationInfo
            );
        }

        /// Get underlying Vulkan handle from held image
        vk::Image Get() const noexcept
        {
            return *m_image;
        }

        /// @brief Get format
        vk::Format Format() const noexcept
        {
            return m_format;
        }

        /// @brief Get 3D extent
        vk::Extent3D Extent() const noexcept
        {
            return m_extent;
        }

        /// Get usage flags
        vk::ImageUsageFlags Usage() const noexcept
        {
            return m_usage;
        }

        /// Get image type
        vk::ImageType ImageType() const noexcept
        {
            return m_imageType;
        }

        /// Get mip levels
        std::uint32_t MipLevels() const noexcept
        {
            return m_mipLevels;
        }

        /// get number of array layers
        std::uint32_t ArrayLayers() const noexcept
        {
            return m_arrayLayers;
        }

        /// Get subresource range
        vk::ImageSubresourceRange GetSubresourceRange(
            vk::ImageAspectFlags aspectMask) const noexcept
        {
            return {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = m_mipLevels,
                .baseArrayLayer = 0,
                .layerCount = m_arrayLayers
            };
        }

        /// Get mutable ImageState ref
        ImageState& State() noexcept
        {
            return m_imageState;
        }

        /// Get immutable ImageState ref
        const ImageState& State() const noexcept
        {
            return m_imageState;
        }

        /// boolean to see if image has been initialized
        explicit operator bool() const noexcept
        {
            return Get() != vk::Image{};
        }

    private:
        /// Internal constructor to allow for factory functions
        Image(
            vma::raii::Image&& image,
            const vk::ImageCreateInfo& info)
            : m_image(std::move(image)),
              m_extent(info.extent),
              m_format(info.format),
              m_usage(info.usage),
              m_imageType(info.imageType),
              m_mipLevels(info.mipLevels),
              m_arrayLayers(info.arrayLayers)
        {
        }

        // ImageResource m_imageResource;
        vma::raii::Image m_image{nullptr};
        Vulkan::ImageState m_imageState{Vulkan::ImageState::Undefined()};

        vk::Extent3D m_extent{};
        vk::Format m_format = vk::Format::eUndefined;
        vk::ImageUsageFlags m_usage{};
        vk::ImageType m_imageType = vk::ImageType::e1D;

        std::uint32_t m_mipLevels = 0;
        std::uint32_t m_arrayLayers = 0;
    };


    // =============================
    // Image view abstraction
    // =============================

    struct ImageViewCreateOptions
    {
        vk::ImageAspectFlags aspectMask{};

        std::uint32_t baseMipLevel = 0;
        std::uint32_t levelCount = 1;

        std::uint32_t baseArrayLayer = 0;
        std::uint32_t layerCount = 1;

        vk::Format format = vk::Format::eUndefined;
    };

    /// vk::ImageView convenience wrapper
    class ImageView
    {
    public:
        ImageView() = default;
        ~ImageView() = default;

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;
        ImageView(ImageView&&) = default;
        ImageView& operator=(ImageView&&) = default;

        // factory function for flexibility
        static ImageView Create(
            const vk::raii::Device& device,
            const Image& image,
            vk::ImageViewCreateInfo info)
        {
            if (!image)
            {
                throw std::invalid_argument(
                    "Cannot create a view of an invalid image.");
            }


            // image handle always comes from the image we want a view to
            info.image = image.Get();

            // image's format unless explicitly overridden
            if (info.format == vk::Format::eUndefined)
                info.format = image.Format();

            vk::raii::ImageView view{device, info};

            return ImageView{std::move(view)};
        }

        static ImageView Create2D(
            const vk::raii::Device& device,
            const Image& image,
            const ImageViewCreateOptions& createOptions)
        {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = createOptions.format;

            createInfo.components = vk::ComponentMapping{
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity
            };

            createInfo.subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = createOptions.aspectMask,
                .baseMipLevel = createOptions.baseMipLevel,
                .levelCount = createOptions.levelCount,
                .baseArrayLayer = createOptions.baseArrayLayer,
                .layerCount = 1 // TODO: handle more layer counts later
            };

            return Create(device, image, createInfo);
        }

        vk::ImageView Get() const noexcept
        {
            return *m_view;
        }

        explicit operator bool() const noexcept
        {
            return Get() != vk::ImageView{};
        }

    private:
        explicit ImageView(vk::raii::ImageView&& view)
            : m_view(std::move(view))
        {
        }

        vk::raii::ImageView m_view{nullptr};
    };

    /// @brief Records one image barrier from `state` to `next` and updates `state`. For work outside the render
    ///        graph (uploads); graph passes get their barriers from the graph.
    inline void TransitionImage(
        vk::CommandBuffer cmd,
        vk::Image image,
        ImageState& state,
        const ImageState& next,
        const vk::ImageSubresourceRange& range)
    {
        const vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = state.stageMask,
            .srcAccessMask = state.accessMask,
            .dstStageMask = next.stageMask,
            .dstAccessMask = next.accessMask,
            .oldLayout = state.layout,
            .newLayout = next.layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = range
        };

        const vk::DependencyInfo dependency{
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        cmd.pipelineBarrier2(dependency);
        state = next;
    }

    /// @brief The same, tracking the state in the image itself (Image::State).
    inline void TransitionImage(
        vk::CommandBuffer commandBuffer,
        Image& image,
        const ImageState& newState,
        vk::ImageAspectFlags aspectMask)
    {
        TransitionImage(
            commandBuffer,
            image.Get(),
            image.State(),
            newState,
            image.GetSubresourceRange(aspectMask));
    }
}
