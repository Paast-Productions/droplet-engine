#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_NO_UNION_CONSTRUCTORS 
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_UNION_CONSTRUCTORS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <stdexcept>
#include <filesystem>
#include <vector>
#include <array>
#include <SDL3/SDL_video.h>

#include "SandboxGeometry.hpp"

namespace Droplet::Sandbox
{
    class SandboxRenderer
    {
    public:
        SandboxRenderer(SDL_Window* window);
        ~SandboxRenderer() noexcept;


        [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
            const std::vector<std::uint32_t>& spirv);
        // TODO: make this part of resources
        /**
         * @brief Create shader modules for vertex and fragment stages
         */
        void CreateShaderModules(
            const std::vector<std::uint32_t>& vertexSpirv,
            const std::vector<std::uint32_t>& fragmentSpirv);

        void WaitIdle();
        void DrawFrame();
        void CreateGraphicsPipeline();
        
        // sets a flag to recreate the swapchain
        void OnResize(){m_frameBufferResized = true;};
    
    private:
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();

        /// @brief Iterate over all available physical devices (GPUs) and find one that supports all required properties
        bool IsDeviceSuitable(const vk::raii::PhysicalDevice& candidate);
        void SelectPhysicalDevice();
        void CreateLogicalDevice();

        static uint32_t ChooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
        static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availableModes);
        vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
        void CreateSwapchain();
        void CreateImageViews();

        void CreateCommandPool();
        void CreateCommandBuffers();
        void TransitionImageLayout(uint32_t imageIndex,
                                   vk::ImageLayout old_layout,
                                   vk::ImageLayout new_layout,
                                   vk::AccessFlags2 src_access_mask,
                                   vk::AccessFlags2 dst_access_mask,
                                   vk::PipelineStageFlags2 src_stage_mask,
                                   vk::PipelineStageFlags2 dst_stage_mask) const;
        void RecordCommandBuffer(std::uint32_t imageIndex) const;

        void CreateSyncObjects();
        void CreatePipelineLayout();
        void CreateAllocator();

        // buffers
        void CreateVertexBuffer();
        
        // utils
        void IncrementFrameIndex(){ m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT; };
        uint32_t FindMemoryType(uint32_t typeFiller, vk::MemoryPropertyFlags properties) const;
                
        // swapchain utils
        void CleanupSwapchain();
        void RecreateSwapchain();
    
    private:
        static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT{2};
        static constexpr std::uint32_t MINIMUM_API_VERSION = vk::ApiVersion13;

        // dependency order top to bottom
        vk::raii::Context m_context;
        vk::raii::Instance m_instance{nullptr};

        std::vector<const char*> m_requiredDeviceExtensions{vk::KHRSwapchainExtensionName};
        const std::vector<char const*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
#endif
        // Rendering surface
        vk::raii::SurfaceKHR m_surface{nullptr};

        // == Device ==
        vk::raii::PhysicalDevice m_physicalDevice{nullptr};
        std::uint32_t m_graphicsPresentQueueFamily = 0;

        vk::raii::Device m_device{nullptr}; ///< Logical device
        vk::raii::Queue m_graphicsQueue{nullptr};

        // == Vma Allocator == 
        vma::raii::Allocator m_allocator{nullptr};

        // == Swapchain ==
        vk::raii::SwapchainKHR m_swapchain{nullptr};
        std::vector<vk::Image> m_swapchainImages; // non-owning handles to swapchain images
        std::vector<vk::raii::ImageView> m_swapchainImageViews;
        vk::SurfaceFormatKHR m_swapchainSurfaceFormat{};
        vk::Extent2D m_swapchainExtent{};

        // == Command Pool ==
        vk::raii::CommandPool m_commandPool{nullptr};
        std::vector<vk::raii::CommandBuffer> m_commandBuffers;

        // == Synchronization resources ==
        std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
        std::vector<vk::raii::Fence> m_inFlightFences;
        uint32_t m_frameIndex{0};

        // == Shaders ==
        vk::raii::ShaderModule m_vertexShaderModule{nullptr};
        vk::raii::ShaderModule m_fragmentShaderModule{nullptr};

        // == Pipeline == 
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};
        vk::raii::Pipeline m_graphicsPipeline{nullptr};
        vk::ClearValue m_defaultClearColor{vk::ClearColorValue{{{0.002f, 0.002f, 0.002f, 1.0f}}}};

        // buffer objects
        const std::vector<Vertex> m_vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        vk::raii::Buffer m_vertexBuffer { nullptr };
        vk::raii::DeviceMemory m_vertexBufferMemory {nullptr};
        
        // keeping a reference to the window instance for convenience
        SDL_Window* m_windowRef{nullptr};
        bool m_frameBufferResized{false};

#ifndef NDEBUG
        vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif
    };
}
