/**
 * @file Swapchain.cpp
 * @brief Vulkan::Swapchain implementation: format, present mode, image count and extent; acquire, present, recreate.
 */
#include "Integration/Vulkan/WSI/Swapchain.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <print>
#include <stdexcept>

#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Core/VulkanUtils.hpp"

// AcquireNextImage and Present rely on eErrorOutOfDateKHR being returned instead of thrown.
#if !defined(VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)
#error "Define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS for the whole target (target_compile_definitions)"
#endif

namespace
{
    /// @brief At least three images, within the surface's limits (maxImageCount 0 means no limit).
    std::uint32_t ChooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        std::uint32_t minImageCount = std::max(3u, capabilities.minImageCount);
        if (0 < capabilities.maxImageCount && capabilities.maxImageCount < minImageCount)
        {
            minImageCount = capabilities.maxImageCount;
        }
        return minImageCount;
    }

    /// @brief B8G8R8A8 sRGB with sRGB non-linear colour space if available, else the first format.
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.empty())
            throw std::runtime_error("The surface reports no formats");

        // TODO: put in a config
        static constexpr auto SC_FORMAT = vk::Format::eR8G8B8A8Unorm;
        static constexpr auto SC_COLOR_SPACE = vk::ColorSpaceKHR::eSrgbNonlinear;
        const auto preferredFormatIt = std::ranges::find_if(
            availableFormats,
            [](const vk::SurfaceFormatKHR& surfaceFormat)
            {
                return surfaceFormat.format == SC_FORMAT && surfaceFormat.colorSpace == SC_COLOR_SPACE;
            });

        return preferredFormatIt != availableFormats.end() ? *preferredFormatIt : availableFormats.front();
    }

    /// @brief Mailbox if available, else FIFO (which every device supports).
    vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availableModes)
    {
        assert(std::ranges::find(availableModes, vk::PresentModeKHR::eFifo) != availableModes.end());

        return std::ranges::find(availableModes, vk::PresentModeKHR::eMailbox) != availableModes.end()
                   ? vk::PresentModeKHR::eMailbox
                   : vk::PresentModeKHR::eFifo;
    }

    /**
     * @brief The surface's current extent, or the framebuffer size clamped to the surface limits.
     * @param capabilities    the surface's capabilities
     * @param surfaceProvider the window system, asked for the framebuffer size in pixels
     * @throws std::runtime_error if the size is 0 x 0 (minimized)
     */
    vk::Extent2D ChooseSwapExtent(
        const vk::SurfaceCapabilitiesKHR& capabilities,
        const Vulkan::ISurfaceProvider& surfaceProvider)
    {
        // the surface specifies the size for us
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
            return capabilities.currentExtent;

        // query the framebuffer size in pixels from the window system
        const vk::Extent2D framebufferExtent = surfaceProvider.GetFramebufferExtent();
        if (framebufferExtent.width == 0 || framebufferExtent.height == 0)
            throw std::runtime_error("Cannot create a swapchain for a zero-sized window.");

        // clamp framebuffer size to surface capabilities
        return vk::Extent2D{
            .width = std::clamp(
                framebufferExtent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width),
            .height = std::clamp(
                framebufferExtent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height)
        };
    }
}

using namespace Vulkan;

Swapchain::Swapchain(const Context& context)
    : m_context{&context}
{
    Create();
}

AcquiredImage Swapchain::AcquireNextImage(const Semaphore& imageAvailable) const
{
    // eErrorOutOfDateKHR is returned, not thrown (VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS); then the
    // semaphore was not signalled
    const auto [result, imageIndex] =
        m_swapchain.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), imageAvailable.Handle());
    switch (result)
    {
    case vk::Result::eSuccess:
        return {.status = SwapchainStatus::Optimal, .image = GetSwapchainImage(imageIndex)};
    case vk::Result::eSuboptimalKHR:
        return {.status = SwapchainStatus::Suboptimal, .image = GetSwapchainImage(imageIndex)};
    case vk::Result::eErrorOutOfDateKHR:
        return {.status = SwapchainStatus::OutOfDate};
    default:
        // eTimeout or eNotReady: cannot happen with an infinite timeout
        Vulkan::Utils::ThrowResult(result, "Failed to acquire a swapchain image");
    }
}

SwapchainImage Swapchain::GetSwapchainImage(std::uint32_t imageIndex) const
{
    return {
        .index = imageIndex,
        .image = GetImage(imageIndex),
        .view = GetImageView(imageIndex),
        .extent = m_extent,
        .format = m_surfaceFormat.format,
        .renderFinished = &GetRenderFinishedSemaphore(imageIndex)
    };
}

SwapchainStatus Swapchain::Present(std::uint32_t imageIndex) const
{
    const vk::Semaphore renderFinished = GetRenderFinishedSemaphore(imageIndex).Handle();
    const vk::SwapchainKHR swapchain = *m_swapchain;

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };

    const vk::Result result = m_context->GetGraphicsQueue().presentKHR(presentInfo);
    Utils::CheckResult(result, "Presentation failed",
                               vk::Result::eSuccess, vk::Result::eSuboptimalKHR, vk::Result::eErrorOutOfDateKHR);

    switch (result)
    {
    case vk::Result::eSuboptimalKHR: return SwapchainStatus::Suboptimal;
    case vk::Result::eErrorOutOfDateKHR: return SwapchainStatus::OutOfDate;
    default: return SwapchainStatus::Optimal;
    }
}

void Swapchain::Recreate()
{
    // The old swapchain is handed to the new one (oldSwapchain) and destroyed once the new one exists.
    Create();
}

void Swapchain::Create()
{
    const vk::SurfaceCapabilitiesKHR capabilities = m_context->GetSurfaceCapabilities();
    m_surfaceFormat = ChooseSwapSurfaceFormat(m_context->GetSurfaceFormats());
    m_presentMode = ChooseSwapPresentMode(m_context->GetSurfacePresentModes());
    m_extent = ChooseSwapExtent(capabilities, m_context->GetSurfaceProvider());

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = *m_context->GetSurface(),
        .minImageCount = ChooseSwapchainMinImageCount(capabilities),
        .imageFormat = m_surfaceFormat.format,
        .imageColorSpace = m_surfaceFormat.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = m_presentMode,
        .clipped = vk::True,
        .oldSwapchain = *m_swapchain // null on the first creation
    };

    // The new swapchain is created from the old one first. Then the old views (of the old images) go, and assigning
    // destroys the old swapchain.
    vk::raii::SwapchainKHR newSwapchain(Device(), createInfo);
    m_imageViews.clear();
    m_swapchain = std::move(newSwapchain);
    m_images = m_swapchain.getImages();

    std::println("Swapchain created: {} images, extents {}x{}", m_images.size(), m_extent.width, m_extent.height);

    CreateImageViews();
    CreateRenderFinishedSemaphores();

    ++m_generation;
}

void Swapchain::CreateImageViews()
{
    m_imageViews.reserve(m_images.size());

    vk::ImageViewCreateInfo viewCreateInfo{
        .image = VK_NULL_HANDLE, // assigned in the loop
        .viewType = vk::ImageViewType::e2D,
        .format = m_surfaceFormat.format,
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    for (const vk::Image image : m_images)
    {
        viewCreateInfo.image = image;
        m_imageViews.emplace_back(Device(), viewCreateInfo);
    }
}

void Swapchain::CreateRenderFinishedSemaphores()
{
    m_renderFinishedSemaphores.clear();
    m_renderFinishedSemaphores.reserve(GetImageCount());

    for (std::uint32_t imageIndex = 0; imageIndex < GetImageCount(); ++imageIndex)
        m_renderFinishedSemaphores.emplace_back(Device(), Semaphore::Type::Binary);
}

const vk::raii::Device& Swapchain::Device() const noexcept
{
    return m_context->GetDevice();
}
