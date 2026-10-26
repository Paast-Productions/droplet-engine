#pragma once

#include <SDL3/SDL_video.h>

#include <cstdint>

#include "SandboxVulkanInstance.hpp"
#include "SandboxVulkanDevice.hpp"
#include "SandboxVulkanSwapchain.hpp"
#include "SandboxRenderResources.hpp"
#include "SandboxFrameResources.hpp"

namespace Droplet::Sandbox
{
    class SandboxRendererLite
    {
    public:
        explicit SandboxRendererLite(SDL_Window* window);

        void WaitIdle();
        void DrawFrame();

        void OnResize()    { m_frameBufferResized = true; }
        void OnRestore()   { m_frameBufferMinimized = false; }
        void OnMinimize()  { m_frameBufferMinimized = true; }

    private:
        void RecordCommandBuffer(std::uint32_t imageIndex) const;
        void UpdateUniformBuffer(std::uint32_t currentImage) const;

        bool m_frameBufferResized{false};
        bool m_frameBufferMinimized{false};

        // abstraction layers
        Vulkan::VulkanInstance m_instance;
        Vulkan::VulkanDevice m_device;
        Vulkan::SandboxSwapchain m_swapchain;
        Vulkan::SandboxRenderResources m_renderResources;
        Vulkan::SandboxFrameResources m_frameResources;
    };
}
