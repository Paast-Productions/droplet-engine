#pragma once

#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <string>

namespace Droplet::Sandbox
{
    class SandboxSDLVulkan
    {
    public:
        static std::vector<const char*> GetRequiredInstanceExtensions()
        {
            Uint32 extensionCount = 0;
            const char* const * sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

            if (sdlExtensions == nullptr)
            {
                throw std::runtime_error(
                    std::string(
                        "SDL_Vulkan_GetInstanceExtensions failed: ") 
                        + SDL_GetError());
            }
    
            return {sdlExtensions, sdlExtensions + extensionCount};
        }

        static VkSurfaceKHR CreateSurface(
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
    };
}