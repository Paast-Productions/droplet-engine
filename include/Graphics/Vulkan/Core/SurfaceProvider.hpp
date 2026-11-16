/**
 * @file SurfaceProvider.hpp
 * @brief wrapper used per-platform to extract what the Vulkan backend needs regardless of platform window system
 */
#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

namespace Droplet::Graphics::Platform
{
    /**
     * @brief The window system's side of presenting: instance extensions, surface creation and framebuffer size.
     *
     * The Vulkan backend (Instance, Surface, Context, WSI) only talks to the window through this interface,
     * so it does not depend on SDL.
     * An implementation/subclass must outlive the Context and WSI created from it.
     */
    class ISurfaceProvider
    {
    public:
        virtual ~ISurfaceProvider() = default;

        /**
         * @brief Instance extensions the window system needs for presenting (VK_KHR_surface plus a platform one)
         * @return extension names as a vector of string. The strings must stay valid as long as the provider
         * @throws std::runtime_error if the window system cannot report them for some reason
         */
        [[nodiscard]] virtual std::vector<const char*> GetRequiredInstanceExtensions() const = 0;

        /**
         * @brief Creates a Vk surface for the window. Caller takes ownership.
         * @param instance instance created with GetRequiredInstanceExtensions() enabled
         * @return the new surface, never VK_NULL_HANDLE
         * @throws std::runtime_error if the surface cannot be created
         */
        [[nodiscard]] virtual VkSurfaceKHR CreateSurface(VkInstance instance) const = 0;

        /**
         * @brief Current drawable size of the window in pixels (NOT window coordinates)
         * @return the size, 0x0 while minimized
         * @throws std::runtime_error if the window system cannot report it
         */
        [[nodiscard]] virtual vk::Extent2D GetFramebufferExtent() const = 0;

    protected:
        ISurfaceProvider() = default;
        ISurfaceProvider(const ISurfaceProvider&) = default;
        ISurfaceProvider& operator=(const ISurfaceProvider&) = default;
    };
}
