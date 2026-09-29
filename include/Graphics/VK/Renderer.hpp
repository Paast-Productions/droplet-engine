#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <Graphics/SDL/Window.hpp>
#include <Graphics/VK/Pipeline.hpp>
#include <optional>

#include <Graphics/VK/IndexBuffer.hpp>
#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/UniformBuffer.hpp>
#include <Graphics/VK/DepthBuffer.hpp>
#include <Graphics/VK/ImageView.hpp>
#include <Graphics/VK/UniformBuffer.hpp>
#include <Graphics/VK/CommandPool.hpp>
#include <Graphics/VK/Swapchain.hpp>
#include <Graphics/VK/VulkanContext.hpp>


// HACK: Implementation subject to change

class Renderer
{
public:
	Renderer() = delete;
	Renderer(Droplet::Graphics::SDL::WindowConfig p_windowConfig);
	~Renderer();
	int		Initialize();
	int		Initialize(const Slang::ComPtr<slang::IBlob>& p_shaderBlob);
	void	drawFrame();
	void	windowResize();

	SDL_Event				p_event {};
	inline static			SDL_InitState p_init {};

private:
	/// @brief Creates the graphics pipeline
	void					CreateGraphicsPipeline();

	/// @brief Creates a graphics pipeline based on a shader
	/// @param p_shaderBlob shader to be used
	void					CreateGraphicsPipeline(const Slang::ComPtr<slang::IBlob> &p_shaderBlob);

	/// @brief Creates a shader module from a vector containing raw code
	/// @param p_device Pointer to the vulkan device
	/// @param code Vector containing raw code
	/// @return A vulkan shader module
	vk::raii::ShaderModule  CreateShaderModule(const vk::raii::Device &p_device, const std::vector<char> &code) const;

	/// @brief Creates a shader module from a blob containing code
	/// @param p_device Pointer to the vulkan device
	/// @param code Blob containing code
	/// @return A vulkan shader module
	vk::raii::ShaderModule  CreateShaderModule(const vk::raii::Device &p_device, const Slang::ComPtr<slang::IBlob> &code) const;
	
	/// @brief Creates the command pool
	void					CreateCommandPool();

	/// @brief Creates commandbuffers
	void					CreateCommandBuffers();

	/// @brief Records a command buffer for rendering an image
	/// @param imageIndex which image to render to
	void					RecordCommandBuffer(uint32_t imageIndex);

	/// @brief Creates the texture sampler
	void					CreateTextureSampler();

	/// @brief Creates the descriptorset layout
	void					CreateDescriptorSetLayout();

	/// @brief Creates the descriptor pool
	void					CreateDescriptorPool();

	/// @brief Creates the descriptorsets
	void					CreateDescriptorSets();

	/// @brief Creates sync objects for preventing race conditions etc
	void					CreateSyncObjects();

	/// @brief Changes the layout of an image from one to another
	/// @param image The image to be translated
	/// @param old_layout The old layout of the image
	/// @param new_layout The new layout of the image 
	/// @param src_access_mask Source access mask
	/// @param dst_access_mask Destination access mask
	/// @param src_stage_mask Source stage mask
	/// @param dst_stage_mask Destination stage mask
	/// @param image_aspect_flags Image aspect flags and/or bits
	void					TransitionImageLayout(
		vk::Image               image,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask,
		vk::ImageAspectFlags    image_aspect_flags);

	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	//Declaration order matters for destruction order!!!!

	std::optional<Droplet::Graphics::VK::VulkanContext> m_context;

	SDL_Event m_event;

	std::optional < Droplet::Graphics::VK::Swapchain> m_swapchain;

	std::optional<Droplet::Graphics::VK::CommandPool> m_commandPool;
	std::vector<Droplet::Graphics::VK::CommandBufferId> m_commandBufferIds;
	std::optional<Droplet::Graphics::VK::Pipeline> m_graphicsPipeline;
	Droplet::Graphics::SDL::Window			m_window;

	std::vector<vk::raii::Semaphore>	 	m_presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore>	 	m_renderFinishedSemaphores;
	std::vector<vk::raii::Fence>		 	m_inFlightFences;

	vk::raii::DescriptorPool			 m_descriptorPool = nullptr;
	vk::raii::DescriptorSetLayout		 m_descriptorSetLayout = nullptr;
	std::vector<vk::raii::DescriptorSet> m_descriptorSets;
	vk::raii::Sampler					 m_textureSampler = nullptr;

	std::optional<Droplet::Graphics::VK::ImageView>	   m_textureView;
	std::optional<Droplet::Graphics::VK::UniformBuffer> m_uniformBuffer;
	std::optional<Droplet::Graphics::VK::DepthBuffer>  m_depthBuffer;
	std::optional<Droplet::Graphics::VK::IndexBuffer>  m_indexBuffer;
	std::optional<Droplet::Graphics::VK::VertexBuffer> m_vertexBuffer;

	//Needs one buffer per frame in flight to avoid read write issues
	std::optional<Droplet::Graphics::VK::UniformBuffer> m_uniformBuffers[MAX_FRAMES_IN_FLIGHT];

	std::uint32_t							 m_frameIndex = 0;

	bool								 m_framebufferResized = false;

	//std::vector<const char*>			 m_requiredDeviceExtension = { vk::KHRSwapchainExtensionName };
	};