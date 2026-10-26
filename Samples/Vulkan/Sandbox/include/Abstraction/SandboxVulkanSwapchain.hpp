#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Sandbox::Vulkan
{
    class SandboxSwapchain
    {
    public:
        SandboxSwapchain();

        void RecreateSwapchain();
    private:
        static uint32_t ChooseSwapchainMinImageCount(
            const vk::SurfaceCapabilitiesKHR& capabilities);

        static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& availableFormats);

        static vk::PresentModeKHR ChooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR>& availableModes);

        vk::Extent2D ChooseSwapExtent(
            const vk::SurfaceCapabilitiesKHR& capabilities) const;

        void CreateSwapchain();
        void CreateImageViews();
        void CleanupSwapchain();
    private:
        vk::raii::SwapchainKHR m_swapchain{nullptr};

        std::vector<vk::Image> m_swapchainImages;
        std::vector<vk::raii::ImageView> m_swapchainImageViews;

        vk::SurfaceFormatKHR m_swapchainSurfaceFormat{};
        vk::Extent2D m_swapchainExtent{};
    };
}
