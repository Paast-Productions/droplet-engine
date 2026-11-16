/**
 * @file Surface.hpp
 * @brief abstraction of VkSurfaceKHR that takes an abstract surface provider. 
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
    class ISurfaceProvider;
    class Instance;
    
    class Surface
    {
    public:
        /**
         * @brief Creates the surface through `surfaceProvider` and takes ownership of it.
         * @param surfaceProvider the window system; must outlive the surface
         * @param instance        the instance to create it with; must outlive the surface
         * @throws std::runtime_error if the window system cannot create the surface
         */
        Surface(const ISurfaceProvider& surfaceProvider, const Instance& instance);
        ~Surface() = default;

        Surface(const Surface&) = delete;
        Surface& operator=(const Surface&) = delete;
        Surface(Surface&&) = delete;
        Surface& operator=(Surface&&) = delete;

        /// @brief The RAII surface used for rendering
        [[nodiscard]] const vk::raii::SurfaceKHR& Get() const noexcept { return m_surface; }

    private:
        vk::raii::SurfaceKHR m_surface{nullptr}; ///< Owned, destroyed before the instance
    };
}
