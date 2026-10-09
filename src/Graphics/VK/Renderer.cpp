#include "Renderer.hpp"

#include <print>
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
#include <chrono>

#include <SDL3/SDL_vulkan.h>
#include <Graphics/VK/UniformBuffer.hpp>
#include <Graphics/VK/TestData.hpp>
#include <GameInput.hpp>

#include <Graphics/VK/BufferHelper.hpp>
#include <tracy/public/tracy/Tracy.hpp>

const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

using namespace Droplet::Graphics;

Renderer::Renderer(SDL::WindowConfig p_windowConfig) : 
	m_window		{ p_windowConfig },
	m_context		{ m_window.Get(), m_vkContext },
	m_allocator		{ m_context.GetInstance(), m_context.GetPhysicalDevice(), m_context.GetDevice() },
	m_commandPool	{ m_context.GetDevice(), m_context.GetQueueIndex(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer, MAX_FRAMES_IN_FLIGHT },
	m_swapchain		{ m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window.Get(), m_context.GetSurface() },
	m_depthBuffer	{ m_allocator.Get(), m_context.GetDevice(), m_context.GetPhysicalDevice(), m_swapchain.GetExtent() },
	m_indexBuffer	{ m_allocator.Get(), G_INDICES },
	m_vertexBuffer	{ m_allocator.Get(), G_VERTICES }
{
	ZoneScoped;

	// TODO: <REFACTOR>
	CreateDescriptorSetLayout();
	
	CreateGraphicsPipeline();

	for (auto &uniformBuffer : m_uniformBuffers)
	{
		const VK::UniformBufferObject ubo
		{
			.model = glm::rotate(glm::mat4(1.0f), 0.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapchain.GetExtent().width) / static_cast<float>(m_swapchain.GetExtent().height), 0.1f, 10.0f)
		};

		uniformBuffer = { m_allocator.Get(), ubo };
	}
	
	m_image = {
		m_allocator.Get(),
		m_commandPool,
		m_context,
		G_CATDESPAIR,
		78400,
		G_CATDIM,
		G_CATDIM,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	//Format is changed from the usual eR8G8B8A8Srbg/unorm
	m_textureView = {
		m_context.GetDevice(),
		m_image.Get(),
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageAspectFlagBits::eColor
	};
	
	CreateTextureSampler();

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateSyncObjects();


	std::array<vk::DescriptorPoolSize, 2> poolSizes
	{
		{
			{ 
				.type = vk::DescriptorType::eSampledImage,
				.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE
			},
			{ 
				.type = vk::DescriptorType::eSampler, 
				.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE
			},
		}
	};

	vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 0,
		.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	for (vk::DescriptorPoolSize &poolSize : poolSizes)
	{
		poolInfo.maxSets += poolSize.descriptorCount;
	}
	m_imGuiPoolSize = poolInfo.maxSets;
	m_imGuiDescriptorPool = vk::raii::DescriptorPool(m_context.GetDevice(), poolInfo);
}

//Idle the device to allow for cleanup of swapchain and destroy window
Renderer::~Renderer()
{
	ZoneScoped;

	m_context.GetDevice().waitIdle();
	m_swapchain.Cleanup(m_context.GetDevice());
}

//File reading function for loading the shader file
static std::vector<char> readFile(const std::string &filename)
{
	ZoneScoped;

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
void Renderer::ResizeWindow()
{
	m_framebufferResized = true;
}

void Renderer::CreateGraphicsPipeline()
{
	ZoneScoped;

	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = CreateShaderModule(m_context.GetDevice(), readFile("slang.spv"));
	VK::PipelineConfig pipelineConfig
	{
		.SwapchainSurfaceFormat = m_swapchain.GetSurfaceFormat()
	};
	pipelineConfig.PipelineLayoutInfo.pSetLayouts = &*m_descriptorSetLayout;
	m_graphicsPipeline = { m_context.GetDevice(), m_context.GetPhysicalDevice(), shaderModule, pipelineConfig };
}

//Main definition of the desired pipeline --> Dynamic state decides what values are allowed to change in runtime
void Renderer::CreateGraphicsPipeline(const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
	ZoneScoped;

	//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("compiled.spv"));
	vk::raii::ShaderModule shaderModule = CreateShaderModule(m_context.GetDevice(), p_shaderBlob);
	VK::PipelineConfig pipelineConfig 
	{
		.SwapchainSurfaceFormat = m_swapchain.GetSurfaceFormat()
	};
	
	m_graphicsPipeline = { m_context.GetDevice(), m_context.GetPhysicalDevice(), shaderModule, pipelineConfig };
}

[[nodiscard]] vk::raii::ShaderModule Renderer::CreateShaderModule(const vk::raii::Device &p_device, const std::vector<char> &p_code) const
{
	ZoneScoped;

	vk::ShaderModuleCreateInfo createInfo
	{
		.codeSize = p_code.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(p_code.data())
	};
	
	vk::raii::ShaderModule     shaderModule
	{
		p_device,
		createInfo
	};

	return shaderModule;
}

//Creation function for shaders
[[nodiscard]] vk::raii::ShaderModule Renderer::CreateShaderModule(const vk::raii::Device &p_device, const Slang::ComPtr<slang::IBlob> &p_shaderBlob) const
{
	ZoneScoped;

	vk::ShaderModuleCreateInfo createInfo{ .codeSize = p_shaderBlob->getBufferSize(), .pCode = static_cast<const std::uint32_t*>(p_shaderBlob->getBufferPointer()) };
	vk::raii::ShaderModule     shaderModule{ p_device, createInfo};

	return shaderModule;
}

//Change data layout of image
void Renderer::TransitionImageLayout(
	vk::Image				p_image,
	vk::ImageLayout         p_oldLayout,
	vk::ImageLayout         p_newLayout,
	vk::AccessFlags2        p_srcAccessMask,
	vk::AccessFlags2        p_dstAccessMask,
	vk::PipelineStageFlags2 p_srcStageMask,
	vk::PipelineStageFlags2 p_dstStageMask,
	vk::ImageAspectFlags    p_imageAspectFlags)
{
	ZoneScoped;

	vk::ImageMemoryBarrier2 barrier
	{
		.srcStageMask = p_srcStageMask,
		.srcAccessMask = p_srcAccessMask,
		.dstStageMask = p_dstStageMask,
		.dstAccessMask = p_dstAccessMask,
		.oldLayout = p_oldLayout,
		.newLayout = p_newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = p_image,
		.subresourceRange = 
		{
			.aspectMask = p_imageAspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		} 
	};
	
	vk::DependencyInfo dependencyInfo
	{
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier 
	};
	
	const vk::raii::CommandBuffer& commandBuffer = m_commandPool.GetBufferAt(m_frameIndex);
	
	commandBuffer.pipelineBarrier2(dependencyInfo);
}


//Main drawing operations are here!
void Renderer::RecordCommandBuffer(uint32_t p_imageIndex)
{
	ZoneScoped;

	auto &commandBuffer = m_commandPool.GetBufferAt(m_frameIndex);

	commandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	TransitionImageLayout(
		m_swapchain.GetImages().at(p_imageIndex),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor
	);

	TransitionImageLayout(
		m_depthBuffer.GetImage(),
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

	vk::RenderingAttachmentInfo colorAttachmentInfo 
	{
		.imageView = m_swapchain.GetImageViews().at(p_imageIndex),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor 
	};

	vk::RenderingAttachmentInfo depthAttachmentInfo
	{
		.imageView = m_depthBuffer.GetView(),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDepth 
	};


	vk::RenderingInfo renderingInfo 
	{
		.renderArea = 
		{
			.offset = 
			{
				.x = 0,
				.y = 0
			}, 
			.extent = m_swapchain.GetExtent()
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo 
	};
	
	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline.Get());
	commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(m_swapchain.GetExtent().height), static_cast<float>(m_swapchain.GetExtent().width), -static_cast<float>(m_swapchain.GetExtent().height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swapchain.GetExtent()));
	commandBuffer.bindVertexBuffers(0, *m_vertexBuffer.Get(), { 0 });
	commandBuffer.bindIndexBuffer(*m_indexBuffer.Get(), 0, vk::IndexType::eUint16);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline.GetLayout(), 0, *m_descriptorSets[m_frameIndex], nullptr);
	commandBuffer.drawIndexed(static_cast<uint32_t>(G_INDICES.size()), 1, 0, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
	commandBuffer.endRendering();
	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	
	TransitionImageLayout(
		m_swapchain.GetImages().at(p_imageIndex),
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
	ZoneScoped;

	assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

	for (size_t i = m_swapchain.GetImages().size(); i > 0; i--)
	{
		m_renderFinishedSemaphores.emplace_back(m_context.GetDevice(), vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_presentCompleteSemaphores.emplace_back(m_context.GetDevice(), vk::SemaphoreCreateInfo());
		m_inFlightFences.emplace_back(m_context.GetDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

//The part that is called in main and handles presenting of frames and swapchain recreation when window is resized 
void Renderer::DrawFrame()
{
	ZoneScoped;

	// Create temporary deltaTime that Update's will use
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

	static TimePoint s_lastFrameTime{ std::chrono::high_resolution_clock::now() };
	const TimePoint  currentTime{ std::chrono::high_resolution_clock::now() };
	const float deltaTime = { std::chrono::duration<float>(currentTime - s_lastFrameTime).count() };
	s_lastFrameTime = currentTime;
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
	//while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = m_context.GetDevice().waitForFences(*m_inFlightFences[m_frameIndex], vk::True, std::numeric_limits<std::uint64_t>::max());
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

	auto [result, imageIndex] = m_swapchain.Get().acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *m_presentCompleteSemaphores[m_frameIndex], nullptr);

	m_result = result;
	m_imageIndex = imageIndex;

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (m_result == vk::Result::eErrorOutOfDateKHR)
	{
		while ((SDL_GetWindowFlags(m_window.Get()) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&m_event);
		}

		m_swapchain.Cleanup(m_context.GetDevice());
		m_swapchain.Recreate(m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window.Get(), m_context.GetSurface());
		m_depthBuffer = {
			m_allocator.Get(),
			m_context.GetDevice(),
			m_context.GetPhysicalDevice(),
			m_swapchain.GetExtent()
		};

		return;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.
	// On any error code, aquireNextImage already threw an exception.
	if (m_result != vk::Result::eSuccess && m_result != vk::Result::eSuboptimalKHR)
	{
		assert(m_result == vk::Result::eTimeout || m_result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	// old code
	float rotation;
	m_cameraController.UpdateCamera(m_camera, deltaTime, m_window);
	
	ImGui::Begin("Camera");
	ImGui::Text("%f", m_camera.GetPosition().x);
	ImGui::Text("%f", m_camera.GetPosition().y);
	ImGui::Text("%f", m_camera.GetPosition().z);
	ImGui::Text("%f", m_camera.GetForward().x);
	ImGui::Text("%f", m_camera.GetForward().y);
	ImGui::Text("%f", m_camera.GetForward().z);
	ImGui::Text("%f", deltaTime);

	ImGui::SliderFloat("Roration", &rotation, 0, 360);
	ImGui::End();

	const auto extent = m_swapchain.GetExtent();

	float aspectRatio =
		static_cast<float>(extent.width) /
		static_cast<float>(extent.height);

	VK::UniformBufferObject ubo
	{
		.model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f)),
		.view = m_camera.GetViewMatrix(),
		.proj = m_camera.GetProjectionMatrix(aspectRatio)
	};
	
	m_uniformBuffers.at(m_frameIndex).UpdateBuffer(ubo);

	// Only reset the fence if we are submitting work
	m_context.GetDevice().resetFences(*m_inFlightFences[m_frameIndex]);

	ImGui::EndFrame();
	ImGui::Render();

	m_commandPool.GetBufferAt(m_frameIndex).reset();
	RecordCommandBuffer(m_imageIndex);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	
	const vk::SubmitInfo   submitInfo
	{ 
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*m_presentCompleteSemaphores[m_frameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*m_commandPool.GetBufferAt(m_frameIndex),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*m_renderFinishedSemaphores[m_imageIndex]
	};
	
	m_context.GetQueue().submit(submitInfo, *m_inFlightFences[m_frameIndex]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*m_renderFinishedSemaphores[m_imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*m_swapchain.Get(),
		.pImageIndices = &m_imageIndex
	};
	
	m_result = m_context.GetQueue().presentKHR(presentInfoKHR);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if ((m_result == vk::Result::eSuboptimalKHR) || (m_result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
	{
		m_framebufferResized = false;
		while ((SDL_GetWindowFlags(m_window.Get()) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&m_event);
		}
		m_swapchain.Cleanup(m_context.GetDevice());
		m_swapchain.Recreate(m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window.Get(), m_context.GetSurface());
		m_depthBuffer = {
			m_allocator.Get(),
			m_context.GetDevice(),
			m_context.GetPhysicalDevice(),
			m_swapchain.GetExtent()
		};
	}
	else
	{
		// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
		assert(m_result == vk::Result::eSuccess);
	}
	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

//Descriptors and samplers

void Renderer::CreateTextureSampler()
{
	ZoneScoped;

	vk::PhysicalDeviceProperties properties = m_context.GetPhysicalDevice().getProperties();
	vk::SamplerCreateInfo        samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::False,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways 
	};
	m_textureSampler = vk::raii::Sampler(m_context.GetDevice(), samplerInfo);
}

//Descriptor pool is increased in size for the sampler
//If the descriptor pool is inadequate it might still pass the validation layers and fail on some machines but not others
void Renderer::CreateDescriptorPool()
{
	ZoneScoped;

	std::array<vk::DescriptorPoolSize, 2> poolSize
	{ 
		{
			{
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = MAX_FRAMES_IN_FLIGHT
			},
			{
				.type = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = MAX_FRAMES_IN_FLIGHT
			}
		} 
	};
	
	vk::DescriptorPoolCreateInfo          poolInfo
	{ 
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
		.pPoolSizes = poolSize.data() 
	};
	
	m_descriptorPool = vk::raii::DescriptorPool(m_context.GetDevice(), poolInfo);
}

//Sets layout of buffer
//Multiple bindings can be created at once, here we added the sampler
void Renderer::CreateDescriptorSetLayout() 
{
	ZoneScoped;

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings
	{
		{
			{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1, 
				.stageFlags = vk::ShaderStageFlagBits::eVertex
			},
			
			//Specify where the sampler is to be used with the ShaderStageFlag
			 {
			 	.binding = 1,
			 	.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			 	.descriptorCount = 1,
			 	.stageFlags = vk::ShaderStageFlagBits::eFragment
			 }
		} 
	};
	
	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.bindingCount = static_cast<uint32_t>(bindings.size()), 
		.pBindings = bindings.data()
	};
	
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(m_context.GetDevice(), layoutInfo);
}

void Renderer::CreateDescriptorSets()
{
	ZoneScoped;

	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo        allocInfo
	{
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data() 
	};

	m_descriptorSets.clear();
	m_descriptorSets = m_context.GetDevice().allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo
		{
			.buffer = m_uniformBuffers[i].Get(), 
			.offset = 0,
			.range = sizeof(VK::UniformBufferObject)
		};
		
		vk::DescriptorImageInfo  imageInfo
		{
			.sampler = m_textureSampler, 
			.imageView = m_textureView.Get(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{ 
		{
			{
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo
			},
																
			{
				.dstSet = m_descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo}
			} 
		};
		m_context.GetDevice().updateDescriptorSets(descriptorWrites, {});
	}
}

static void CheckVkResult(VkResult p_err)
{
	if (p_err == VK_SUCCESS)
		return;
	throw ("[vulkan] Error: VkResult = %d\n", p_err);
}

void Renderer::GetImGuiInitInfo(ImGui_ImplVulkan_InitInfo &p_initInfo)
{
	ZoneScoped;

	//TODO: CREATE PIPELINE CACHE

	p_initInfo.Instance = *m_context.GetInstance();
	p_initInfo.PhysicalDevice = *m_context.GetPhysicalDevice();
	p_initInfo.Device = *m_context.GetDevice();
	p_initInfo.QueueFamily = m_context.GetQueueIndex();
	p_initInfo.Queue = *m_context.GetQueue();
	p_initInfo.PipelineCache = VK_NULL_HANDLE;
	p_initInfo.DescriptorPool = *m_imGuiDescriptorPool;
	p_initInfo.MinImageCount = 2;
	p_initInfo.ImageCount = 2;
	p_initInfo.Allocator = nullptr;
	p_initInfo.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
	p_initInfo.PipelineInfoMain.Subpass = 0;
	p_initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	p_initInfo.CheckVkResultFn = CheckVkResult;

	p_initInfo.UseDynamicRendering = VK_TRUE;

	m_imGuiColorFormat = static_cast<VkFormat>(m_swapchain.GetSurfaceFormat().format);
	VkFormat depthFormat = static_cast<VkFormat>(m_depthBuffer.GetFormat());

	p_initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	p_initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	p_initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_imGuiColorFormat;
	p_initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
	p_initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	p_initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	p_initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	p_initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_imGuiColorFormat;
	p_initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
	p_initInfo.PipelineInfoForViewports.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
}

SDL_Window *Renderer::GetWindow()
{
	return m_window.Get();
}

void Renderer::WaitIdle()
{
	ZoneScoped;

	m_context.GetDevice().waitIdle();
}