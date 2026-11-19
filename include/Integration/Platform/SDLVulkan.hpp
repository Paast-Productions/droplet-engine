#pragma once

#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <string>

#include "Integration/Vulkan/Core/SurfaceProvider.hpp"

namespace SDL
{
    /// Get required vulkan extensions in order to use an SDL_Window as a VkSurface
    inline std::vector<const char*> GetRequiredInstanceExtensions()
    {
        Uint32 extensionCount = 0;
        const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

        if (sdlExtensions == nullptr)
        {
            throw std::runtime_error(
                std::string(
                    "SDL_Vulkan_GetInstanceExtensions failed: ")
                + SDL_GetError());
        }

        return { sdlExtensions, sdlExtensions + extensionCount };
    }

    /// Create a vulkan surface from an SDL window
    inline VkSurfaceKHR CreateSurface(
        SDL_Window* window,
        VkInstance instance)
    {
        VkSurfaceKHR surface{};

        if (!SDL_Vulkan_CreateSurface(
            window, instance, nullptr, &surface))
        {
            throw std::runtime_error(
                std::string("SDL_Vulkan_CreateSurface failed: ")
                + SDL_GetError());
        }

        return surface;
    }

    class VulkanSurfaceProvider final : public Vulkan::ISurfaceProvider
    {
    public:
        /**
         * @param window window created with SDL_WINDOW_VULKAN
         * @throws std::invalid_argument if window is null
         */
        explicit VulkanSurfaceProvider(SDL_Window* window)
            : m_window{ window }
        {
            if (m_window == nullptr)
                throw std::invalid_argument("SDLVulkanSurface: window was null");
        }

        /// @copydoc
        /// 
        ///  Vulkan::ISurfaceProvider::GetRequiredInstanceExtensions
        [[nodiscard]] std::vector<const char*> GetRequiredInstanceExtensions() const override
        {
            return SDL::GetRequiredInstanceExtensions();
        }

        /// @copydoc Vulkan::ISurfaceProvider::CreateSurface
        [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance) const override
        {
            return SDL::CreateSurface(m_window, instance);
        }

        /// @copydoc Vulkan::ISurfaceProvider::GetFramebufferExtent
        [[nodiscard]] vk::Extent2D GetFramebufferExtent() const override
        {
            int width = 0;
            int height = 0;
            if (!SDL_GetWindowSizeInPixels(m_window, &width, &height))
            {
                throw std::runtime_error(
                    std::string("SDL_GetWindowSizeInPixels failed: ")
                    + SDL_GetError());
            }

            return vk::Extent2D{
                .width = width > 0 ? static_cast<std::uint32_t>(width) : 0u,
                .height = height > 0 ? static_cast<std::uint32_t>(height) : 0u
            };
        }

    private:
        SDL_Window* m_window{ nullptr }; ///< Not owned.
    };
}
