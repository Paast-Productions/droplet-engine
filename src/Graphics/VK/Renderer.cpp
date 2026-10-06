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
	m_context->GetDevice()->waitIdle();
	m_swapchain->Cleanup(*m_context->GetDevice());
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

void Renderer::CreateGraphicsPipeline()
{
	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = CreateShaderModule(*m_context->GetDevice(), readFile("../../src/Graphics/VK/Shaders/slang.spv"));
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchain->GetSurfaceFormat()};
	pipelineConfig.PipelineLayoutInfo.pSetLayouts = &*m_descriptorSetLayout;
	m_graphicsPipeline.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), shaderModule, pipelineConfig);
}

//Main definition of the desired pipeline --> Dynamic state decides what values are allowed to change in runtime
void Renderer::CreateGraphicsPipeline(const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = CreateShaderModule(*m_context->GetDevice(), p_shaderBlob);
	Droplet::Graphics::VK::PipelineConfig pipelineConfig = { .SwapchainSurfaceFormat = m_swapchain->GetSurfaceFormat() };
	m_graphicsPipeline.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), shaderModule, pipelineConfig);
}

[[nodiscard]] vk::raii::ShaderModule Renderer::CreateShaderModule(const vk::raii::Device &p_device, const std::vector<char> &p_code) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = p_code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(p_code.data()) };
	vk::raii::ShaderModule     shaderModule{ p_device, createInfo};

	return shaderModule;
}

//Creation function for shaders
[[nodiscard]] vk::raii::ShaderModule Renderer::CreateShaderModule(const vk::raii::Device &p_device, const Slang::ComPtr<slang::IBlob> &p_shaderBlob) const
{
	vk::ShaderModuleCreateInfo createInfo{ .codeSize = p_shaderBlob->getBufferSize(), .pCode = static_cast<const std::uint32_t*>(p_shaderBlob->getBufferPointer()) };
	vk::raii::ShaderModule     shaderModule{ p_device, createInfo};

	return shaderModule;
}

//Needed for creation of commandBuffers
void Renderer::CreateCommandPool()
{
	m_commandPool.emplace(*m_context->GetDevice(), m_context->GetQueueIndex(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
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
void Renderer::RecordCommandBuffer(uint32_t p_imageIndex)
{
	assert(m_graphicsPipeline.has_value());
	
	auto &commandBuffer = m_commandPool->GetBuffer(m_commandBufferIds[m_frameIndex]).Get(); // TODO: Make this get the wrapper, rather than the raw RAII object
	commandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	TransitionImageLayout(
		m_swapchain->GetImages()->at(p_imageIndex),
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
		.imageView = m_swapchain->GetImageViews()->at(p_imageIndex),
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
		m_swapchain->GetImages()->at(p_imageIndex),
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
void Renderer::CreateSyncObjects()
{
	assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

	for (size_t i = m_swapchain->GetImages()->size(); i > 0; i--)
	{
		m_renderFinishedSemaphores.emplace_back(*m_context->GetDevice(), vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_presentCompleteSemaphores.emplace_back(*m_context->GetDevice(), vk::SemaphoreCreateInfo());
		m_inFlightFences.emplace_back(*m_context->GetDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

//The part that is called in main and handles presenting of frames and swapchain recreation when window is resized 
void Renderer::drawFrame()
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
		//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = m_context->GetDevice()->waitForFences(*m_inFlightFences[m_frameIndex], vk::True, UINT64_MAX);
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

		m_swapchain->Cleanup(*m_context->GetDevice());
		m_swapchain->Recreate(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), *m_window.Get(), *m_context->GetSurface());
		m_depthBuffer.reset();
		m_depthBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_swapchain->GetExtent());
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
	m_context->GetDevice()->resetFences(*m_inFlightFences[m_frameIndex]);

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
	m_context->GetQueue()->submit(submitInfo, *m_inFlightFences[m_frameIndex]);

	const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1,
											.pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
											.swapchainCount = 1,
											.pSwapchains = &**m_swapchain->GetSwapchain(),
											.pImageIndices = &imageIndex };
	result = m_context->GetQueue()->presentKHR(presentInfoKHR);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
	{
		m_framebufferResized = false;
		while ((SDL_GetWindowFlags(m_window.Get()) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&m_event);
		}
		m_swapchain->Cleanup(*m_context->GetDevice());
		m_swapchain->Recreate(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), *m_window.Get(), *m_context->GetSurface());
		m_depthBuffer.reset();
		m_depthBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_swapchain->GetExtent());
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
	vk::PhysicalDeviceProperties properties = m_context->GetPhysicalDevice()->getProperties();
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
	m_textureSampler = vk::raii::Sampler(*m_context->GetDevice(), samplerInfo);
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
	m_descriptorPool = vk::raii::DescriptorPool(*m_context->GetDevice(), poolInfo);
}

//Sets layout of buffer
//Multiple bindings can be created at once, here we added the sampler
void Renderer::CreateDescriptorSetLayout() {
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
			{{.binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex},
			//Specify where the sampler is to be used with the ShaderStageFlag
			 {.binding = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment}} };
	vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() };
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(*m_context->GetDevice(), layoutInfo);
}

void Renderer::CreateDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo        allocInfo{
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data() };

	m_descriptorSets.clear();
	m_descriptorSets = m_context->GetDevice()->allocateDescriptorSets(allocInfo);

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
		m_context->GetDevice()->updateDescriptorSets(descriptorWrites, {});
	}
}

//Creation of renderer
int Renderer::Initialize()
{
	try
	{
		m_context.emplace(*m_window.Get());
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

	m_swapchain.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), *m_window.Get(), *m_context->GetSurface());

	CreateDescriptorSetLayout();

	CreateCommandPool();

	CreateGraphicsPipeline();

	m_depthBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_swapchain->GetExtent());

	m_vertexBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_VERTICES);
	m_indexBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_INDICES);

	//Format is changed from the usual eR8G8B8A8Srbg/unorm
	m_textureView.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_CATDESPAIR, G_CATDIM, G_CATDIM, vk::Format::eR5G6B5UnormPack16, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

	CreateTextureSampler();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i].emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice());
	}

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	CreateSyncObjects();

	return 0;
}

int Renderer::Initialize(const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
	try
	{
		m_context.emplace(*m_window.Get());
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

	m_swapchain.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), *m_window.Get(), *m_context->GetSurface());

	CreateDescriptorSetLayout();

	CreateCommandPool();

	CreateGraphicsPipeline(p_shaderBlob);

	m_depthBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_swapchain->GetExtent());

	m_vertexBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_VERTICES);
	m_indexBuffer.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_INDICES);

	//Format is changed from the usual eR8G8B8A8Srbg/unorm
	m_textureView.emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice(), m_commandPool.value(), *m_context->GetQueue(), G_CATDESPAIR, G_CATDIM, G_CATDIM, vk::Format::eR5G6B5UnormPack16, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

	CreateTextureSampler();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i].emplace(*m_context->GetDevice(), *m_context->GetPhysicalDevice());
	}

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	CreateSyncObjects();

	return 0;
}
