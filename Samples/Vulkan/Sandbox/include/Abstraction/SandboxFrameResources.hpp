#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Sandbox::Vulkan
{
    class SandboxFrameResources
    {
    public:
        SandboxFrameResources(vk::raii::Device &device)
        {
            CreateCommandPool();
            CreateCommandBuffers();
            CreateSyncObjects();
        }
        
        void IncrementFrameIndex();
        uint32_t GetFrameIndex()const{return m_frameIndex; }
    private:
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSyncObjects();
    private:
        vk::raii::CommandPool m_commandPool{nullptr};
        std::vector<vk::raii::CommandBuffer> m_commandBuffers;

        std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
        std::vector<vk::raii::Fence> m_inFlightFences;

        uint32_t m_frameIndex{0};
    };
}
