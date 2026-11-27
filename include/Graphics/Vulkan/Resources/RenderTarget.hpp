/**
 * @file RenderTarget.hpp
 * @author Sebastian Lorensson
 * @brief Vulkan::RenderTarget: a colour image and its view that is drawn into and sampled; Resize recreates it.
 * @date 2026-11-02
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Abstraction/Image.hpp"

#include <stdexcept>
#include <utility>

namespace Droplet::Graphics::Vulkan
{
    struct RenderTargetCreateInfo
    {
        Image2DCreateInfo image;
        ImageViewCreateOptions view{
            .aspectMask = vk::ImageAspectFlagBits::eColor
        };
    };

    class RenderTarget
    {
    public:
        RenderTarget() = default;
        ~RenderTarget() = default;

        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        RenderTarget(RenderTarget&& other) noexcept
            :
            m_image(std::move(other.m_image)),
            m_imageView(std::move(other.m_imageView)),
            m_createInfo(other.m_createInfo)
        {
        }

        RenderTarget& operator=(RenderTarget&& other) noexcept
        {
            if (this == &other)
                return *this;

            // Destroy the old view before replacing its image
            m_imageView = ImageView{};

            m_imageView = std::move(other.m_imageView); // old view dies before the old image
            m_image = std::move(other.m_image);
            m_createInfo = other.m_createInfo;
            return *this;
        }

        static RenderTarget Create(
            const vma::raii::Allocator& allocator,
            const vk::raii::PhysicalDevice& physicalDevice,
            const vk::raii::Device& device,
            const RenderTargetCreateInfo& createInfo)
        {
            constexpr vk::Flags<vk::ImageUsageFlagBits> requiredUsage =
                vk::ImageUsageFlagBits::eColorAttachment
                | vk::ImageUsageFlagBits::eSampled;
            if ((createInfo.image.usage & requiredUsage) != requiredUsage)
                throw std::invalid_argument("Render target images must support color attachment and sampled usage");

            // query physical device format support
            const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(createInfo.image.format);

            constexpr vk::FormatFeatureFlags requiredFeatures =
                vk::FormatFeatureFlagBits::eSampledImage
                | vk::FormatFeatureFlagBits::eColorAttachment
                | vk::FormatFeatureFlagBits::eSampledImageFilterLinear;

            const vk::FormatFeatureFlags availableFeatures =
                createInfo.image.tiling == vk::ImageTiling::eOptimal
                    ? formatProperties.optimalTilingFeatures
                    : formatProperties.linearTilingFeatures;

            if ((availableFeatures & requiredFeatures) != requiredFeatures)
            {
                throw std::runtime_error(
                    "Image format does not support required render target operations");
            }

            Image image = Image::Create2D(allocator, createInfo.image);
            ImageView view = ImageView::Create2D(device, image, createInfo.view);

            return {std::move(image), std::move(view), createInfo};
        }

        bool Resize(
            const vma::raii::Allocator& allocator,
            const vk::raii::PhysicalDevice& physicalDevice,
            const vk::raii::Device& device,
            vk::Extent2D newExtent)
        {
            // Vulkan images cannot have zero-sized dimensions.
            if (newExtent.width == 0 || newExtent.height == 0)
                return false;

            // Nothing to do if the extent hasn't changed.
            const vk::Extent2D currentExtent = Extent();

            if (currentExtent.width == newExtent.width &&
                currentExtent.height == newExtent.height)
            {
                return false;
            }

            // Reuse the existing creation information.
            RenderTargetCreateInfo createInfo = m_createInfo;

            createInfo.image.extent = vk::Extent2D{
                .width = newExtent.width,
                .height = newExtent.height,
            };

            // Construct the replacement before destroying the old resources.
            RenderTarget replacement = Create(
                allocator,
                physicalDevice,
                device,
                createInfo);

            // Replace the old resources.
            *this = std::move(replacement);

            return true;
        }

        vk::Image GetImage() const noexcept { return m_image.Get(); }
        vk::ImageView GetImageView() const noexcept { return m_imageView.Get(); }

        vk::Extent2D Extent() const noexcept
        {
            vk::Extent3D extent = m_image.Extent();
            return {.width = extent.width, .height = extent.height};
        }

        vk::Format Format() const noexcept { return m_image.Format(); }

        Vulkan::ImageState& State() noexcept
        {
            return m_image.State();
        }

        const Vulkan::ImageState& State() const noexcept
        {
            return m_image.State();
        }

        Image& GetImageResource() noexcept { return m_image; }
        const Image& GetImageResource() const noexcept { return m_image; }
        const ImageView& GetImageViewResource() const noexcept { return m_imageView; }


        explicit operator bool() const noexcept
        {
            return
                static_cast<bool>(m_image) &&
                static_cast<bool>(m_imageView);
        }

    private:
        RenderTarget(
            Image&& image,
            ImageView&& view,
            const RenderTargetCreateInfo& info)
            :
            m_image(std::move(image)),
            m_imageView(std::move(view)),
            m_createInfo(info)
        {
        }

        Image m_image;
        ImageView m_imageView;

        RenderTargetCreateInfo m_createInfo;
    };
}
