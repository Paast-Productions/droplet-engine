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

#include <Graphics/VK/CommandPool.hpp>


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
	void					createInstance();
	void					setupDebugMessenger();
	void					createSurface();
	bool					isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	void					pickPhysicalDevice();
	void					createLogicalDevice();
	vk::SurfaceFormatKHR	chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
	vk::PresentModeKHR		chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
	vk::Extent2D			chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
	uint32_t				chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
	void					createSwapChain();
	void					createImageViews();
	void					createGraphicsPipeline();
	void					createGraphicsPipeline(const Slang::ComPtr<slang::IBlob> &p_shaderBlob);
	vk::raii::ShaderModule  createShaderModule(const std::vector<char> &code) const;
	vk::raii::ShaderModule  createShaderModule(const Slang::ComPtr<slang::IBlob> &code) const;
	void					CreateCommandPool();
	void					CreateCommandBuffers();
	void					RecordCommandBuffer(uint32_t imageIndex);

	void					createSyncObjects();

	void					recreateSwapChain();
	void					cleanupSwapChain();

	void					transition_image_layout(
		uint32_t                imageIndex,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask);

	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	//Declaration order matters for destruction order!!!!
	vk::raii::Context						m_context;
	vk::raii::Instance						m_instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT		m_debugMessenger = nullptr;
	vk::raii::SurfaceKHR					m_surface = nullptr;
	vk::raii::PhysicalDevice				m_physicalDevice = nullptr;
	vk::raii::Device						m_device = nullptr;
	std::uint32_t							m_queueIndex = static_cast<std::uint32_t>(~0);
	vk::raii::Queue							m_queue = nullptr;
	vk::raii::SwapchainKHR					m_swapchain = nullptr;
	std::vector<vk::Image>					m_swapchainImages	{};
	vk::SurfaceFormatKHR					m_swapchainSurfaceFormat;
	vk::Extent2D							m_swapchainExtent;
	std::vector<vk::raii::ImageView>		m_swapchainImageViews;

	vk::raii::PipelineLayout				m_pipelineLayout	= nullptr;
	std::optional<Droplet::Graphics::VK::CommandPool> m_commandPool;
	std::vector<Droplet::Graphics::VK::CommandBufferId> m_commandBufferIds;
	std::optional<Droplet::Graphics::VK::Pipeline> m_graphicsPipeline;
	Droplet::Graphics::SDL::Window			m_window;

	std::vector<vk::raii::Semaphore>	 	m_presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore>	 	m_renderFinishedSemaphores;
	std::vector<vk::raii::Fence>		 	m_inFlightFences;

	std::optional<Droplet::Graphics::VK::DepthBuffer> m_depthBuffer;
	std::optional<Droplet::Graphics::VK::IndexBuffer> m_indexBuffer;
	std::optional<Droplet::Graphics::VK::VertexBuffer> m_vertexBuffer;

	//Needs one buffer per frame in flight to avoid read write issues
	std::optional<Droplet::Graphics::VK::UniformBuffer> m_uniformBuffers[MAX_FRAMES_IN_FLIGHT];

	std::uint32_t							 m_frameIndex = 0;

	bool								 m_framebufferResized = false;

	std::vector<const char*>			 m_requiredDeviceExtension = { vk::KHRSwapchainExtensionName };
	};