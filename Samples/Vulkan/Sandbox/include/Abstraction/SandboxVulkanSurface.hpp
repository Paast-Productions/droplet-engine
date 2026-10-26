#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "SandboxSDLVulkan.hpp"
#include "SandboxVulkanInstance.hpp"


namespace Droplet::Sandbox::Vulkan
{
    class VulkanSurface
    {
    public:
        VulkanSurface(SDL_Window* p_window, const VulkanInstance &p_instance)
        {
            assert(p_window != nullptr && "Window was invalid.");

            VkSurfaceKHR rawSurface = SandboxSDLVulkan::CreateSurface(p_window, *p_instance.Get());
        
            // give ownership of surface to owned raii surface
            m_surface = vk::raii::SurfaceKHR(p_instance.Get(), rawSurface);
        }

        ~VulkanSurface() = default;
    private:
        vk::raii::SurfaceKHR m_surface{nullptr};
    };
}
