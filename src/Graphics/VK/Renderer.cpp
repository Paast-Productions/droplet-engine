#include "Renderer.hpp"

#include <map>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cstdint> // Necessary for uint32_t
#include <filesystem>
#include <string>

#include <SDL3/SDL_vulkan.h>


const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif


//testing values
std::vector<Droplet::Graphics::VK::Vertex> g_vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},{0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f},{0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f},{1.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},{0.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f},{0.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f}}
};


const std::vector<uint16_t> g_indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4 };

Renderer::Renderer(Droplet::Graphics::SDL::WindowConfig p_windowConfig) :
	m_window{p_windowConfig} {}

//Idle the device to allow for cleanup of swapchain and destroy window
Renderer::~Renderer()
{
	m_device.waitIdle();
	cleanupSwapChain();

}

//File reading function for loading the shader file
static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();
	return buffer;
}

//Called to notify the renderer of a window resizing event
void Renderer::windowResize()
{
	m_framebufferResized = true;
}

//Fetches required SDL instance extensions
std::vector<const char*> getRequiredInstanceExtensions()
{
	uint32_t extensionCount = 0;
	auto     sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

	std::vector extensions(sdlExtensions, sdlExtensions + extensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

//Initializing the vulkan instance
void Renderer::createInstance()
{
	constexpr vk::ApplicationInfo appInfo{ .pApplicationName = "Hello Triangle!",
										   .applicationVersion = VK_MAKE_VERSION(1,0,0),
										   .pEngineName = "No Engine",
										   .engineVersion = VK_MAKE_VERSION(1,0,0),
										   .apiVersion = vk::ApiVersion14 };

	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = m_context.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
		[&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}

	// Get the required extensions.
	auto requiredExtensions = getRequiredInstanceExtensions();

	// Check if the required extensions are supported by the Vulkan implementation.
	auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
	auto unsupportedPropertyIt =
		std::ranges::find_if(requiredExtensions,
			[&extensionProperties](auto const& requiredExtension) {
				return std::ranges::none_of(extensionProperties,
					[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
			});
	if (unsupportedPropertyIt != requiredExtensions.end())
	{
		throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
	}

	vk::InstanceCreateInfo createInfo{ .pApplicationInfo = &appInfo,
									  .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
									  .ppEnabledLayerNames = requiredLayers.data(),
									  .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
									  .ppEnabledExtensionNames = requiredExtensions.data() };
	m_instance = vk::raii::Instance(m_context, createInfo);
}

//Debug stuff, I didn't really touch it
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

//Setup of the debug messenger
void Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{ .messageSeverity = severityFlags,
																		  .messageType = messageTypeFlags,
																		  .pfnUserCallback = &debugCallback };
	m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

//Creating a surface for rendering onto
void Renderer::createSurface()
{
	VkSurfaceKHR _surface;
	if (!SDL_Vulkan_CreateSurface(m_window.Get(), *m_instance, nullptr, &_surface))
	{
		throw std::runtime_error("failed to create window surface!");
	}
	m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
}

//Iterating through a list of available GPUs and choosing one to use
void Renderer::pickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
	auto const                            devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	m_physicalDevice = *devIter;
}

//Creation of a vulkan device
void Renderer::createLogicalDevice() 
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

	// get the first index into queueFamilyProperties which supports both graphics and present
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface))
		{
			// found a queue family that supports both graphics and present
			m_queueIndex = qfpIndex;
			break;
		}
	}
	if (m_queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},									   // vk::PhysicalDeviceFeatures2
			{.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
			{.synchronization2 = true, .dynamicRendering = true}, // vk::PhysicalDeviceVulkan13Features   //Fixes sync2 warnings
			{.extendedDynamicState = true},        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	};

	// create a Device
	float                     queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = m_queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	vk::DeviceCreateInfo      deviceCreateInfo{ .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
											   .queueCreateInfoCount = 1,
											   .pQueueCreateInfos = &deviceQueueCreateInfo,
											   .enabledExtensionCount = static_cast<uint32_t>(m_requiredDeviceExtension.size()),
											   .ppEnabledExtensionNames = m_requiredDeviceExtension.data() };

	m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
	m_queue = vk::raii::Queue(m_device, m_queueIndex, 0);
}

//Checking if the device supports the correct features and API version
bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_3;

	// Check if any of the queue families support both graphics and presentation to our surface
	auto     queueFamilies = physicalDevice.getQueueFamilyProperties();
	uint32_t qfpIndex = 0;
	bool     supportsGraphicsAndPresent =
		std::ranges::any_of(queueFamilies,
			[&physicalDevice, &surface = this->m_surface, &qfpIndex](auto const& qfp) {
				bool const suitable = (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface);
				qfpIndex++;
				return suitable;
			});

	// Check if all required physicalDevice extensions are available
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(m_requiredDeviceExtension,
			[&availableDeviceExtensions](auto const& requiredDeviceExtension) {
				return std::ranges::any_of(availableDeviceExtensions,
					[requiredDeviceExtension](auto const& availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
			});

	// Check if the physicalDevice supports the required features
	auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_3 && supportsGraphicsAndPresent && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

//Choosing the color format of the surface for the swapchain
vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) 
{
	const auto formatIt = std::ranges::find_if(
		availableFormats,
		[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

//I don't really remember
uint32_t Renderer::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

//Creation of the swapchain
void Renderer::createSwapChain() 
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);
	m_swapchainExtent = chooseSwapExtent(surfaceCapabilities);
	uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
	m_swapchainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR(*m_surface);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{ .surface = *m_surface,
											   .minImageCount = minImageCount,
											   .imageFormat = m_swapchainSurfaceFormat.format,
											   .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
											   .imageExtent = m_swapchainExtent,
											   .imageArrayLayers = 1,
											   .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
											   .imageSharingMode = vk::SharingMode::eExclusive,
											   .preTransform = surfaceCapabilities.currentTransform,
											   .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
											   .presentMode = chooseSwapPresentMode(availablePresentModes),
											   .clipped = true };

	m_swapchain = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
	m_swapchainImages = m_swapchain.getImages();
}

//Choose the size of the surface to be swapped --> Could be wrong
vk::Extent2D Renderer::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
	// currentExtent is only set to the special "undefined" value described above
	// when the window manager lets us choose the extent ourselves; any other value
	// means the surface already dictates a fixed extent that we must use as-is.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	int width, height;
	SDL_GetWindowSize(m_window.Get(), &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

//Clean the buffer and destroy the swapchain
void Renderer::cleanupSwapChain()
{
	m_swapchainImageViews.clear();
	m_swapchain = nullptr;
}

//Cleaning and creating a new swapchain and respective image views
void Renderer::recreateSwapChain()
{
	int width = 0, height = 0;
	SDL_GetWindowSize(m_window.Get(), &width, &height);
	while ((width == 0 || height == 0) && !SDL_ShouldQuit(&p_init)) {
		SDL_GetWindowSize(m_window.Get(), &width, &height);
		SDL_WaitEvent(&p_event);
	}
	if (SDL_ShouldQuit(&p_init)) {
		SDL_SetInitialized(&p_init, false);
		return;
	}

	m_device.waitIdle();

	cleanupSwapChain();
	createSwapChain();
	createImageViews();
}

//Use eMailbox to avoid tearing while still maintaining fairly low latency by rendering new images that are as up to date as possible right until the vertical blank (higher energy usage?)
vk::PresentModeKHR Renderer::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
{
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(availablePresentModes,
		[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
		vk::PresentModeKHR::eMailbox :
		vk::PresentModeKHR::eFifo;
}

//Create image views for the swap chain surfaces --> Maybe like shader resourceviews
void Renderer::createImageViews()
{
	assert(m_swapchainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType = vk::ImageViewType::e2D,
												.format = m_swapchainSurfaceFormat.format,
												.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1} };
	for (auto& image : m_swapchainImages)
	{
		imageViewCreateInfo.image = image;
		m_swapchainImageViews.emplace_back(m_device, imageViewCreateInfo);
	}
}

void Renderer::createGraphicsPipeline()
{
	std::cout << std::filesystem::current_path().generic_string() << std::endl;

	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = createShaderModule(readFile("../../src/Graphics/VK/Shaders/slang.spv"));
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchainSurfaceFormat };
	m_graphicsPipeline.emplace(m_device, shaderModule, pipelineConfig);
}

//Main definition of the desired pipeline --> Dynamic state decides what values are allowed to change in runtime
void Renderer::createGraphicsPipeline(const Slang::ComPtr<slang::IBlob>& p_shaderBlob)
{
	std::cout << std::filesystem::current_path().generic_string() << std::endl;

	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = createShaderModule(p_shaderBlob);
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchainSurfaceFormat };
	m_graphicsPipeline.emplace(m_device, shaderModule, pipelineConfig);
}

[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const std::vector<char>& code) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
	vk::raii::ShaderModule     shaderModule{ m_device, createInfo };

	return shaderModule;
}

//Creation function for shaders
[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const Slang::ComPtr<slang::IBlob>& p_shaderBlob) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = p_shaderBlob->getBufferSize(), .pCode = static_cast<const std::uint32_t*>(p_shaderBlob->getBufferPointer()) };
	vk::raii::ShaderModule     shaderModule{ m_device, createInfo };

	return shaderModule;
}

//Needed for creation of commandBuffers
void Renderer::createCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
									   .queueFamilyIndex = m_queueIndex };
	m_commandPool = vk::raii::CommandPool(m_device, poolInfo);
}

//One buffer per frame in flight
void Renderer::createCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = m_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT };
	m_commandBuffers = vk::raii::CommandBuffers(m_device, allocInfo);
}

//Defines new layout for images?
void Renderer::transition_image_layout(
	uint32_t                imageIndex,
	vk::ImageLayout         old_layout,
	vk::ImageLayout         new_layout,
	vk::AccessFlags2        src_access_mask,
	vk::AccessFlags2        dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask,
	vk::PipelineStageFlags2 dst_stage_mask)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = src_stage_mask,
		.srcAccessMask = src_access_mask,
		.dstStageMask = dst_stage_mask,
		.dstAccessMask = dst_access_mask,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = m_swapchainImages[imageIndex],
		.subresourceRange = {
			   .aspectMask = vk::ImageAspectFlagBits::eColor,
			   .baseMipLevel = 0,
			   .levelCount = 1,
			   .baseArrayLayer = 0,
			   .layerCount = 1} };
	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier };

	m_commandBuffers[m_frameIndex].pipelineBarrier2(dependency_info);
}

//Main drawing operations are here!
void Renderer::recordCommandBuffer(uint32_t imageIndex)
{
	assert(m_graphicsPipeline.has_value());
	
	auto& _commandBuffer = m_commandBuffers[m_frameIndex];
	_commandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
	);
	vk::ClearValue              clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = m_swapchainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor };
	vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = m_swapchainExtent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo };

	_commandBuffer.beginRendering(renderingInfo);
	_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline->Get());
	_commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_swapchainExtent.width), static_cast<float>(m_swapchainExtent.height), 0.0f, 1.0f));
	_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swapchainExtent));
	_commandBuffer.draw(3, 1, 0, 0);
	_commandBuffer.endRendering();

	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
	);
	_commandBuffer.end();
}

//Need to create fences and semaphores for each frame in flight
//Creation of objects for parallellization, semaphores for GPU, fences for CPU
void Renderer::createSyncObjects()
{
	assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		m_renderFinishedSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_presentCompleteSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo());
		m_inFlightFences.emplace_back(m_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

//The part that is called in main and handles presenting of frames and swapchain recreation when window is resized 
void Renderer::drawFrame()
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
		//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = m_device.waitForFences(*m_inFlightFences[m_frameIndex], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

	auto [result, imageIndex] = m_swapchain.acquireNextImage(UINT64_MAX, *m_presentCompleteSemaphores[m_frameIndex], nullptr);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapChain();
		return;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.
	// On any error code, aquireNextImage already threw an exception.
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Only reset the fence if we are submitting work
	m_device.resetFences(*m_inFlightFences[m_frameIndex]);

	m_commandBuffers[m_frameIndex].reset();
	recordCommandBuffer(imageIndex);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo   submitInfo{ .waitSemaphoreCount = 1,
									  .pWaitSemaphores = &*m_presentCompleteSemaphores[m_frameIndex],
									  .pWaitDstStageMask = &waitDestinationStageMask,
									  .commandBufferCount = 1,
									  .pCommandBuffers = &*m_commandBuffers[m_frameIndex],
									  .signalSemaphoreCount = 1,
									  .pSignalSemaphores = &*m_renderFinishedSemaphores[imageIndex] };
	m_queue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

	const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1,
											.pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
											.swapchainCount = 1,
											.pSwapchains = &*m_swapchain,
											.pImageIndices = &imageIndex };
	result = m_queue.presentKHR(presentInfoKHR);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
	{
		m_framebufferResized = false;
		recreateSwapChain();
	}
	else
	{
		// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
		assert(result == vk::Result::eSuccess);
	}
	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}


//Creation of renderer
int Renderer::Initialize()
{
	try
	{
		createInstance();
	}
	catch (const vk::SystemError& err)
	{
		std::cerr << "Vulkan Error: " << err.what() << std::endl;
		return 1;
	}
	catch (const std::exception& err)
	{
		std::cerr << "Error: " << err.what() << std::endl;
		return 1;
	}

	setupDebugMessenger();

	createSurface();

	pickPhysicalDevice();

	createLogicalDevice();

	createSwapChain();

	createImageViews();

	createGraphicsPipeline();

	createCommandPool();

	createCommandBuffers();

	createSyncObjects();

	return 0;
}

int Renderer::Initialize(const Slang::ComPtr<slang::IBlob>& p_shaderBlob)
{
	try
	{
		createInstance();
	}
	catch (const vk::SystemError& err)
	{
		std::cerr << "Vulkan Error: " << err.what() << std::endl;
		return 1;
	}
	catch (const std::exception& err)
	{
		std::cerr << "Error: " << err.what() << std::endl;
		return 1;
	}

	setupDebugMessenger();

	createSurface();

	pickPhysicalDevice();

	createLogicalDevice();

	createSwapChain();

	createImageViews();

	createGraphicsPipeline(p_shaderBlob);

	createCommandPool();

	m_depthBuffer.emplace(m_device, m_physicalDevice, m_swapchainExtent);

	m_vertexBuffer.emplace(m_device, m_physicalDevice, m_commandPool, m_queue, g_vertices);
	m_indexBuffer.emplace(m_device, m_physicalDevice, m_commandPool, m_queue, g_indices);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i].emplace(m_device, m_physicalDevice);
	}

	createCommandBuffers();

	createSyncObjects();

	return 0;
}
