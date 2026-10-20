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

namespace Droplet::Sandbox
{
	class SandboxRenderer
	{
	public:
		SandboxRenderer(SDL_Window* window);
		~SandboxRenderer();


		// TODO: make this part of resources
		/**
		 * @brief Create shader modules for vertex and fragment stages
		 */
		void CreateShaderModules(
			const std::vector<std::uint32_t>& vertexSpirv,
			const std::vector<std::uint32_t>& fragmentSpirv);

		void DrawFrame();
		void CreateGraphicsPipeline();
	private:
		void CreateInstance();
		void CreateSurface();
		/// @brief Iterate over all available physical devices (GPUs) and find one that supports all required properties
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		vk::SurfaceFormatKHR ChooseSurfaceFormat(
			const std::vector<vk::SurfaceFormatKHR>& formats);
		vk::Extent2D ChooseSwapExtent(
			const vk::SurfaceCapabilitiesKHR& capabilities);
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandResources();
		void RecordClearCommandBuffer(std::uint32_t imageIndex) const;
		void CreateSyncObjects();
		void CreatePipelineLayout();
		void CreateAllocator();
	private:
		static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };
		static constexpr std::uint32_t MINIMUM_API_VERSION = vk::ApiVersion13;

		// dependency order top to bottom
		vk::raii::Context m_context;
		vk::raii::Instance m_instance{ nullptr };
		vk::raii::SurfaceKHR m_surface{ nullptr };

		// == Device ==
		vk::raii::PhysicalDevice m_physicalDevice{ nullptr };
		std::uint32_t m_graphicsPresentQueueFamily = 0;

		vk::raii::Device m_device{ nullptr };   ///< Logical device
		vk::raii::Queue m_graphicsPresentQueue{ nullptr };

		// == Vma Allocator == 
		vma::raii::Allocator m_allocator{ nullptr };

		// == Swapchain ==
		vk::raii::SwapchainKHR m_swapchain{ nullptr };
		std::vector<vk::Image> m_swapchainImages; // non-owning handles to swapchain images
		std::vector<vk::raii::ImageView> m_swapchainImageViews;
		vk::SurfaceFormatKHR m_swapchainSurfaceFormat{};
		vk::Extent2D m_swapchainExtent{};

		// == Render Pass ==
		vk::raii::RenderPass m_renderPass{ nullptr };

		// == Frame Buffers ==
		std::vector<vk::raii::Framebuffer> m_swapchainFramebuffers;

		// == Command Pool ==
		vk::raii::CommandPool m_commandPool{ nullptr };
		vk::raii::CommandBuffer m_commandBuffer{ nullptr };

		// == Synchronization resources ==
		vk::raii::Semaphore m_imageAvailableSemaphore{ nullptr };
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		vk::raii::Fence m_inFlightFence{ nullptr };


		// == Shaders ==
		// move these
		vk::raii::ShaderModule m_vertexShaderModule{ nullptr };
		vk::raii::ShaderModule m_fragmentShaderModule{ nullptr };

		// == Pipeline == 
		vk::raii::PipelineLayout m_pipelineLayout{ nullptr };
		vk::raii::Pipeline m_graphicsPipeline{ nullptr };
		vk::ClearValue m_defaultClearColor { vk::ClearColorValue{{{0.002f, 0.002f, 0.002f, 1.0f}}} };

		// keeping a reference to the window instance for convenience
		SDL_Window* m_windowRef{ nullptr };
	};

}