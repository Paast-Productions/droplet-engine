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
#include <Graphics/VK/UniformBuffer.hpp>
#include <Graphics/VK/TestData.hpp>


const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

Renderer::Renderer(Droplet::Graphics::SDL::WindowConfig p_windowConfig) :
	m_window{p_windowConfig} {}

//Idle the device to allow for cleanup of swapchain and destroy window
Renderer::~Renderer()
{
	m_device.waitIdle();
	m_swapchain->Cleanup(m_device);
}

//File reading function for loading the shader file
static std::vector<char> readFile(const std::string &filename)
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
		[&layerProperties](auto const &requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
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
			[&extensionProperties](auto const &requiredExtension) {
				return std::ranges::none_of(extensionProperties,
					[requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
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
	auto const                            devIter = std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice) { return isDeviceSuitable(physicalDevice); });
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
		if ((queueFamilyProperties[qfpIndex].queueFlags  &vk::QueueFlagBits::eGraphics) &&
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
bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
{
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_3;

	// Check if any of the queue families support both graphics and presentation to our surface
	auto     queueFamilies = physicalDevice.getQueueFamilyProperties();
	uint32_t qfpIndex = 0;
	bool     supportsGraphicsAndPresent =
		std::ranges::any_of(queueFamilies,
			[&physicalDevice, &surface = this->m_surface, &qfpIndex](auto const &qfp) {
				bool const suitable = (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface);
				qfpIndex++;
				return suitable;
			});

	// Check if all required physicalDevice extensions are available
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(m_requiredDeviceExtension,
			[&availableDeviceExtensions](auto const &requiredDeviceExtension) {
				return std::ranges::any_of(availableDeviceExtensions,
					[requiredDeviceExtension](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
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

void Renderer::createGraphicsPipeline()
{
	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = createShaderModule(readFile("../../src/Graphics/VK/Shaders/slang.spv"));
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchain->GetSurfaceFormat()};
	pipelineConfig.PipelineLayoutInfo.pSetLayouts = &*m_descriptorSetLayout;
	m_graphicsPipeline.emplace(m_device, m_physicalDevice, shaderModule, pipelineConfig);
}

//Main definition of the desired pipeline --> Dynamic state decides what values are allowed to change in runtime
void Renderer::createGraphicsPipeline(const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = createShaderModule(p_shaderBlob);
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchain->GetSurfaceFormat() };
	m_graphicsPipeline.emplace(m_device, m_physicalDevice, shaderModule, pipelineConfig);
}

[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const std::vector<char> &code) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
	vk::raii::ShaderModule     shaderModule{ m_device, createInfo };

	return shaderModule;
}

//Creation function for shaders
[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const Slang::ComPtr<slang::IBlob> &p_shaderBlob) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = p_shaderBlob->getBufferSize(), .pCode = static_cast<const std::uint32_t*>(p_shaderBlob->getBufferPointer()) };
	vk::raii::ShaderModule     shaderModule{ m_device, createInfo };

	return shaderModule;
}

//Needed for creation of commandBuffers
void Renderer::CreateCommandPool()
{
	m_commandPool.emplace(m_device, m_queueIndex, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
}

//One buffer per frame in flight
void Renderer::CreateCommandBuffers()
{
	assert(m_commandPool.has_value());
	m_commandBufferIds = m_commandPool.value().Allocate(MAX_FRAMES_IN_FLIGHT);
}

//Change data layout of image
void Renderer::TransitionImageLayout(
	vk::Image               image,
	vk::ImageLayout         old_layout,
	vk::ImageLayout         new_layout,
	vk::AccessFlags2        src_access_mask,
	vk::AccessFlags2        dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask,
	vk::PipelineStageFlags2 dst_stage_mask,
	vk::ImageAspectFlags    image_aspect_flags)
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
		.image = image,
		.subresourceRange = {
			.aspectMask = image_aspect_flags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1} };
	vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier };
	Droplet::Graphics::VK::CommandBufferId id;
	id.Index = m_frameIndex;
	m_commandPool->GetBuffer(id).PipelineBarrier(dependencyInfo);
}


//Main drawing operations are here!
void Renderer::RecordCommandBuffer(uint32_t imageIndex)
{
	assert(m_graphicsPipeline.has_value());
	
	auto &commandBuffer = m_commandPool->GetBuffer(m_commandBufferIds[m_frameIndex]).Get(); // TODO: Make this get the wrapper, rather than the raw RAII object
	commandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	TransitionImageLayout(
		m_swapchain->GetImages()->at(imageIndex),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor
	);

	TransitionImageLayout(
		*m_depthBuffer->GetImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth
	);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachmentInfo = {
		.imageView = m_swapchain->GetImageViews()->at(imageIndex),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor };

	vk::RenderingAttachmentInfo depthAttachmentInfo = {
		.imageView = *m_depthBuffer.value().GetView(),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDepth };


	vk::RenderingInfo renderingInfo = 
	{
		.renderArea = {.offset = {0, 0}, .extent = m_swapchain->GetExtent()},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo 
	};

	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline->Get());
	commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(m_swapchain->GetExtent().height), static_cast<float>(m_swapchain->GetExtent().width), -static_cast<float>(m_swapchain->GetExtent().height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swapchain->GetExtent()));
	commandBuffer.bindVertexBuffers(0, **m_vertexBuffer->GetVertexBuffer(), { 0 });
	commandBuffer.bindIndexBuffer(**m_indexBuffer->GetIndexBuffer(), 0, vk::IndexType::eUint16);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline->GetLayout(), 0, *m_descriptorSets[m_frameIndex], nullptr);
	commandBuffer.drawIndexed(static_cast<uint32_t>(G_INDICES.size()), 1, 0, 0, 0);
	commandBuffer.endRendering();

	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	TransitionImageLayout(
		m_swapchain->GetImages()->at(imageIndex),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe,
		vk::ImageAspectFlagBits::eColor
	);
	commandBuffer.end();
}

//Need to create fences and semaphores for each frame in flight
//Creation of objects for parallellization, semaphores for GPU, fences for CPU
void Renderer::createSyncObjects()
{
	assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

	for (size_t i = m_swapchain->GetImages()->size(); i > 0; i--)
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

	auto [result, imageIndex] = m_swapchain->GetSwapchain()->acquireNextImage(UINT64_MAX, *m_presentCompleteSemaphores[m_frameIndex], nullptr);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		while ((SDL_GetWindowFlags(m_window.Get()) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&m_event);
		}

		m_swapchain->Cleanup(m_device);
		m_swapchain->Recreate(m_device, m_physicalDevice, *m_window.Get(), m_surface);
		m_depthBuffer.reset();
		m_depthBuffer.emplace(m_device, m_physicalDevice, m_swapchain->GetExtent());
		return;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.
	// On any error code, aquireNextImage already threw an exception.
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	m_uniformBuffers[m_frameIndex]->UpdateBuffer(m_swapchain->GetExtent());

	// Only reset the fence if we are submitting work
	m_device.resetFences(*m_inFlightFences[m_frameIndex]);

	m_commandPool->GetBuffer(m_commandBufferIds[m_frameIndex]).Get().reset();
	RecordCommandBuffer(imageIndex);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo   submitInfo{ .waitSemaphoreCount = 1,
									  .pWaitSemaphores = &*m_presentCompleteSemaphores[m_frameIndex],
									  .pWaitDstStageMask = &waitDestinationStageMask,
									  .commandBufferCount = 1,
									  .pCommandBuffers = &*m_commandPool->GetBuffer(m_commandBufferIds[m_frameIndex]).Get(),
									  .signalSemaphoreCount = 1,
									  .pSignalSemaphores = &*m_renderFinishedSemaphores[imageIndex] };
	m_queue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

	const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1,
											.pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
											.swapchainCount = 1,
											.pSwapchains = &**m_swapchain->GetSwapchain(),
											.pImageIndices = &imageIndex };
	result = m_queue.presentKHR(presentInfoKHR);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
	{
		m_framebufferResized = false;
		while ((SDL_GetWindowFlags(m_window.Get()) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&m_event);
		}
		m_swapchain->Cleanup(m_device);
		m_swapchain->Recreate(m_device, m_physicalDevice, *m_window.Get(), m_surface);
		m_depthBuffer.reset();
		m_depthBuffer.emplace(m_device, m_physicalDevice, m_swapchain->GetExtent());
	}
	else
	{
		// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
		assert(result == vk::Result::eSuccess);
	}
	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

//Descriptors and samplers

void Renderer::CreateTextureSampler()
{
	vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
	vk::SamplerCreateInfo        samplerInfo{ .magFilter = vk::Filter::eLinear,
											 .minFilter = vk::Filter::eLinear,
											 .mipmapMode = vk::SamplerMipmapMode::eLinear,
											 .addressModeU = vk::SamplerAddressMode::eRepeat,
											 .addressModeV = vk::SamplerAddressMode::eRepeat,
											 .addressModeW = vk::SamplerAddressMode::eRepeat,
											 .mipLodBias = 0.0f,
											 .anisotropyEnable = vk::False,
											 .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
											 .compareEnable = vk::False,
											 .compareOp = vk::CompareOp::eAlways };
	m_textureSampler = vk::raii::Sampler(m_device, samplerInfo);
}

//Descriptor pool is increased in size for the sampler
//If the descriptor pool is inadequate it might still pass the validation layers and fail on some machines but not others
void Renderer::CreateDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 2> poolSize{ {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT},
												{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = MAX_FRAMES_IN_FLIGHT}} };
	vk::DescriptorPoolCreateInfo          poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
												   .maxSets = MAX_FRAMES_IN_FLIGHT,
												   .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
												   .pPoolSizes = poolSize.data() };
	m_descriptorPool = vk::raii::DescriptorPool(m_device, poolInfo);
}

//Sets layout of buffer
//Multiple bindings can be created at once, here we added the sampler
void Renderer::CreateDescriptorSetLayout() {
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
			{{.binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex},
			//Specify where the sampler is to be used with the ShaderStageFlag
			 {.binding = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment}} };
	vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() };
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(m_device, layoutInfo);
}

void Renderer::CreateDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo        allocInfo{
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data() };

	m_descriptorSets.clear();
	m_descriptorSets = m_device.allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo{ .buffer = *m_uniformBuffers[i].value().GetBuffer(), .offset = 0, .range = sizeof(Droplet::Graphics::VK::UniformBufferObject) };
		vk::DescriptorImageInfo  imageInfo{ .sampler = m_textureSampler, .imageView = *m_textureView.value().GetView(), .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{ 
		{
			{
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo},
																
			{
				.dstSet = m_descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo}
			} 
		};
		m_device.updateDescriptorSets(descriptorWrites, {});
	}
}

//Creation of renderer
int Renderer::Initialize()
{
	try
	{
		createInstance();
	}
	catch (const vk::SystemError &err)
	{
		std::cerr << "Vulkan Error: " << err.what() << std::endl;
		return 1;
	}
	catch (const std::exception &err)
	{
		std::cerr << "Error: " << err.what() << std::endl;
		return 1;
	}

	setupDebugMessenger();

	createSurface();

	pickPhysicalDevice();

	createLogicalDevice();

	m_swapchain.emplace(m_device, m_physicalDevice, *m_window.Get(), m_surface);

	CreateDescriptorSetLayout();

	CreateCommandPool();

	createGraphicsPipeline();

	m_depthBuffer.emplace(m_device, m_physicalDevice, m_swapchain->GetExtent());

	m_vertexBuffer.emplace(m_device, m_physicalDevice, m_commandPool.value(), m_queue, G_VERTICES);
	m_indexBuffer.emplace(m_device, m_physicalDevice, m_commandPool.value(), m_queue, G_INDICES);

	//Format is changed from the usual eR8G8B8A8Srbg/unorm
	m_textureView.emplace(m_device, m_physicalDevice, m_commandPool.value(), m_queue, G_CATDESPAIR, G_CATDIM, G_CATDIM, vk::Format::eR5G6B5UnormPack16, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

	CreateTextureSampler();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i].emplace(m_device, m_physicalDevice);
	}

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	createSyncObjects();

	return 0;
}

int Renderer::Initialize(const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
	try
	{
		createInstance();
	}
	catch (const vk::SystemError &err)
	{
		std::cerr << "Vulkan Error: " << err.what() << std::endl;
		return 1;
	}
	catch (const std::exception &err)
	{
		std::cerr << "Error: " << err.what() << std::endl;
		return 1;
	}

	setupDebugMessenger();

	createSurface();

	pickPhysicalDevice();

	createLogicalDevice();

	m_swapchain.emplace(m_device, m_physicalDevice, *m_window.Get(), m_surface);

	createGraphicsPipeline(p_shaderBlob);

	CreateCommandPool();

	m_depthBuffer.emplace(m_device, m_physicalDevice, m_swapchain->GetExtent());
	
	m_vertexBuffer.emplace(m_device, m_physicalDevice, m_commandPool.value(), m_queue, G_TOEVERTICES);
	m_indexBuffer.emplace(m_device, m_physicalDevice, m_commandPool.value(), m_queue, G_TOEINDICES);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i].emplace(m_device, m_physicalDevice);
	}

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	createSyncObjects();

	return 0;
}
