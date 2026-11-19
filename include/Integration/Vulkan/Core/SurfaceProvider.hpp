/**
 * @file SurfaceProvider.hpp
 * @brief Vulkan::ISurfaceProvider: what the Vulkan backend needs from a window system (SDL, GLFW, Win32, ...).
 */
#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

namespace Vulkan
{
    /**
     * @brief The window system's side of presenting: instance extensions, surface creation and framebuffer size.
     *
     * The Vulkan backend (Instance, Surface, Context, WSI) only talks to the window through this interface,
     * so it does not depend on SDL. Implementations live in Platform/ (e.g. SDL::VulkanSurfaceProvider).
     * An implementation must outlive the Context and WSI created from it.
     */
    class ISurfaceProvider
    {
    public:
        virtual ~ISurfaceProvider() = default;

        /**
         * @brief Instance extensions the window system needs for presenting (VK_KHR_surface plus a platform one).
         * @return extension names; the strings must stay valid as long as the provider
         * @throws std::runtime_error if the window system cannot report them
         */
        [[nodiscard]] virtual std::vector<const char*> GetRequiredInstanceExtensions() const = 0;

        /**
         * @brief Creates a surface for the window. The caller takes ownership.
         * @param instance instance created with GetRequiredInstanceExtensions() enabled
         * @return the new surface, never VK_NULL_HANDLE
         * @throws std::runtime_error if the surface cannot be created
         */
        [[nodiscard]] virtual VkSurfaceKHR CreateSurface(VkInstance instance) const = 0;

        /**
         * @brief Current drawable size of the window in pixels (not window coordinates).
         * @return the size; 0 x 0 while the window is minimized
         * @throws std::runtime_error if the window system cannot report it
         */
        [[nodiscard]] virtual vk::Extent2D GetFramebufferExtent() const = 0;

    protected:
        ISurfaceProvider() = default;
        ISurfaceProvider(const ISurfaceProvider&) = default;
        ISurfaceProvider& operator=(const ISurfaceProvider&) = default;
    };
}
