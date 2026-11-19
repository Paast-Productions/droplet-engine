/**
 * @file Surface.cpp
 * @brief Surface implementation: the window surface, created through ISurfaceProvider (SDL today).
 */
#include "Integration/Vulkan/Abstraction/Surface.hpp"

#include <print>
#include <stdexcept>

#include "Integration/Vulkan/Core/SurfaceProvider.hpp"
#include "Integration/Vulkan/Abstraction/Instance.hpp"

namespace Vulkan
{
    Surface::Surface(const ISurfaceProvider& surfaceProvider, const Instance& instance)
    {
        VkSurfaceKHR rawSurface = VK_NULL_HANDLE;
        try
        {
            rawSurface = surfaceProvider.CreateSurface(*instance.Get());
        }
        catch (const std::exception& exception)
        {
            std::println("{} threw while creating raw surface: {}", __func__, exception.what());
            throw;
        }

        if (rawSurface == VK_NULL_HANDLE)
            throw std::runtime_error("Surface: the surface provider returned VK_NULL_HANDLE");

        // give ownership of surface to owned raii surface
        m_surface = vk::raii::SurfaceKHR(instance.Get(), rawSurface);
    }
}
