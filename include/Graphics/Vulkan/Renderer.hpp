#pragma once

#include <vulkan/vulkan_raii.hpp>

#ifdef _EDITOR
#include <ImGui/imgui_impl_vulkan.h>
#endif

#include <Graphics/Vulkan/CommandPool.hpp>
#include <Graphics/Vulkan/Swapchain.hpp>
#include <Graphics/Vulkan/Context.hpp>

namespace Droplet
{
	class Engine;
}

namespace Droplet::Graphics
{
	struct RenderData;
	struct CameraRenderData;
	class RenderTarget;
}

// HACK: Implementation subject to change

namespace Droplet::Graphics
{	
	class Renderer
	{
	public:
		/// @brief Deleted Constructor
		Renderer() = delete;

		/// @brief Renderer Constructor
		/// @param p_window Requested SDL Window Configuration
		Renderer(SDL_Window* p_window);

		/// @brief Destructor
		~Renderer();
		
		/// @brief Draws a list of cameras and their corresponding render data
		void Draw(std::vector<CameraRenderData> p_renderData);
		
		/// @brief Resizes the window
		void ResizeWindow();
		
		SDL_Event				Event {};
		inline static			SDL_InitState s_Init {};
		
	private:
		
		/// @brief Records a command buffer for rendering an image
		/// @param p_renderData Vector of RenderData
		/// @param p_imageIndex Which image to render to
		void RecordRenderTargetCommandBuffer(
			vk::raii::CommandBuffer &p_commandBuffer, 
			const RenderTarget &p_renderTarget, 
			const Camera &p_camera, 
			std::vector<RenderData> &p_renderData, 
			bool p_debug, 
			glm::vec4 p_clearColor);
				
		/// @brief Creates sync objects for preventing race conditions etc
		void CreateSyncObjects();

		/// @brief Changes the layout of an image from one to another
		/// @param p_image The image to be translated
		/// @param p_old_layout The old layout of the image
		/// @param p_new_layout The new layout of the image 
		/// @param p_src_access_mask Source access mask
		/// @param p_dst_access_mask Destination access mask
		/// @param p_src_stage_mask Source stage mask
		/// @param p_dst_stage_mask Destination stage mask
		/// @param p_image_aspect_flags Image aspect flags and/or bits
		/// @param p_commandBuffer Vulkan Command Buffer
		void TransitionImageLayout(
			vk::Image				p_image,
			vk::ImageLayout         p_old_layout,
			vk::ImageLayout         p_new_layout,
			vk::AccessFlags2        p_src_access_mask,
			vk::AccessFlags2        p_dst_access_mask,
			vk::PipelineStageFlags2 p_src_stage_mask,
			vk::PipelineStageFlags2 p_dst_stage_mask,
			vk::ImageAspectFlags    p_image_aspect_flags,
			const vk::raii::CommandBuffer &p_commandBuffer);


		// MAIN RENDERER
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SDL_Window* m_window { nullptr };
		vk::raii::Context m_vkContext;
		Vulkan::Context m_context { nullptr };
		Vulkan::CommandPool m_commandPool { nullptr };
		Vulkan::Swapchain m_swapchain { nullptr };

		std::vector<vk::raii::Semaphore>	 	m_presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore>	 	m_renderFinishedSemaphores;
		std::vector<vk::raii::Fence>		 	m_inFlightFences;
		
		std::uint32_t m_vpBufferIndex { Droplet::G_INVALID_INDEX };
		
		bool m_framebufferResized { false };
		std::uint8_t m_frameIndex { 0 }; // TODO: <REFACTOR /> Should probably be refactored together with sync objects 
		
		// DEBUG
		vk::raii::DescriptorPool m_debugDescriptorPool { nullptr };

#ifdef _EDITOR
		void RecordImGuiCommandBuffer(vk::raii::CommandBuffer &p_commandBuffer, uint32_t p_imageIndex);

		/// @brief Fills and returns initInfo for ImGui
		/// @return ImGui init info
		void GetImGuiInitInfo(ImGui_ImplVulkan_InitInfo &p_initInfo);

		// IMGUI 
		vk::raii::DescriptorPool m_imGuiDescriptorPool { nullptr };
		std::uint32_t m_imGuiPoolSize { 0 };

		VkFormat m_imGuiColorFormat {};
#endif
	};
}