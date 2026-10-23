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

#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyVulkan.hpp>

#include <Graphics/VK/TestData.hpp>
#include <Graphics/DebugDraw.hpp>
#include <GameInput.hpp>
#include <Time.hpp>

const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

using namespace Droplet::Graphics;

Renderer::Renderer(SDL_Window *p_window) :
	m_window							{ p_window },
	m_context							{ m_window, m_vkContext},
	m_commandPool						{ m_context.GetDevice(), m_context.GetQueueIndex(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer, MAX_FRAMES_IN_FLIGHT}
{
	ZoneScoped;
	
	VK::IntermediaryStorage::GetInstance().Initialise(m_context);
	m_swapchain = { VK::IntermediaryStorage::GetInstance().GetAllocator(), m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window, m_context.GetSurface() };
	
	// TODO: <REFACTOR>
	
	// - - - - - - - - - - //
	//         Debug       //
	// - - - - - - - - - - //
	
	vk::DescriptorPoolSize poolSize
	{
		.type = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 2
	};
	
	vk::DescriptorPoolCreateInfo debugPoolInfo
	{ 
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 3,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize 
	};
	
	m_debugDescriptorPool = vk::raii::DescriptorPool(m_context.GetDevice(), debugPoolInfo);
	
	Debug::DebugDraw::Get().Initialize(
		m_context.GetInstance(),
		m_context.GetPhysicalDevice(),
		m_context.GetDevice(),
		VK::IntermediaryStorage::GetInstance().GetAllocator(),
		m_context.GetQueue(),
		m_commandPool.Get(),
		m_debugDescriptorPool,
		m_swapchain.GetSurfaceFormat().format,
		m_swapchain.GetDepthFormat(),
		MAX_FRAMES_IN_FLIGHT);

	const VK::ViewProjectionBuffer vpb
	{
		.view = glm::mat4(1),
		.proj = glm::mat4(1)
	};
	
	m_vpBuffer = { VK::IntermediaryStorage::GetInstance().GetAllocator(), vpb };

	VK::IntermediaryStorage::GetInstance().GetDescriptorSet(vk::DescriptorType::eUniformBuffer).WriteDescriptor(m_context.GetDevice(), m_vpBuffer.Get(), 0);

	CreateSyncObjects();

	// - - - - - - - - - - //
	//         ImGui       //
	// - - - - - - - - - - //
	
	std::array<vk::DescriptorPoolSize, 2> imGuiPoolSizes
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

	vk::DescriptorPoolCreateInfo imGuiPoolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 0,
		.poolSizeCount = static_cast<std::uint32_t>(imGuiPoolSizes.size()),
		.pPoolSizes = imGuiPoolSizes.data()
	};

	for (vk::DescriptorPoolSize &imGuiPoolSize : imGuiPoolSizes)
	{
		imGuiPoolInfo.maxSets += imGuiPoolSize.descriptorCount;
	}
	
	m_imGuiPoolSize = imGuiPoolInfo.maxSets;
	m_imGuiDescriptorPool = vk::raii::DescriptorPool(m_context.GetDevice(), imGuiPoolInfo);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	ImGui_ImplSDL3_InitForVulkan(m_window);
	ImGui_ImplVulkan_InitInfo initInfo {};
	GetImGuiInitInfo(initInfo);
	ImGui_ImplVulkan_Init(&initInfo);
}

//Idle the device to allow for cleanup of swapchain and destroy window
Renderer::~Renderer()
{
	ZoneScoped;
	
	m_context.GetDevice().waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	Debug::DebugDraw::Get().Shutdown();
	VK::IntermediaryStorage::GetInstance().Shutdown();
}

//Called to notify the renderer of a window resizing event
void Renderer::ResizeWindow()
{
	m_framebufferResized = true;
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
	vk::ImageAspectFlags    p_imageAspectFlags,
	const vk::raii::CommandBuffer &p_commandBuffer)
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
	
	//const vk::raii::CommandBuffer& commandBuffer = m_commandPool.GetBufferAt(m_frameIndex);
	auto &commandBuffer = p_commandBuffer;
	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void Renderer::RecordRenderTargetCommandBuffer(vk::raii::CommandBuffer &p_commandBuffer, RenderTarget &p_renderTarget, const Camera &p_camera, std::vector<RenderData> &p_renderData)
{
	ZoneScoped;

	auto &commandBuffer = p_commandBuffer;//m_commandPool.GetBufferAt(m_frameIndex);

	//commandBuffer.begin({});

	TransitionImageLayout(
		p_renderTarget.GetColorImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor,
		commandBuffer
	);


	TransitionImageLayout(
		p_renderTarget.GetDepthImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth,
		commandBuffer
	);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachmentInfo
	{
		.imageView = p_renderTarget.GetColorImageView(),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	vk::RenderingAttachmentInfo depthAttachmentInfo
	{
		.imageView = p_renderTarget.GetDepthImageView(),
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
			.extent = p_renderTarget.GetExtent()
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo
	};

	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.setViewport(0, vk::Viewport(0.0f, static_cast<float>(p_renderTarget.GetExtent().height), static_cast<float>(p_renderTarget.GetExtent().width), -static_cast<float>(p_renderTarget.GetExtent().height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), p_renderTarget.GetExtent()));

	for (const RenderData& data : p_renderData)
	{

		const VK::Pipeline& pipeline = VK::IntermediaryStorage::GetInstance().GetPipeline(data, m_context.GetDevice(), m_context.GetPhysicalDevice(), m_swapchain.GetSurfaceFormat());
		if (pipeline == nullptr) { continue; }

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.Get());
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetLayout(), 0,  VK::IntermediaryStorage::GetInstance().GetDescriptorSets(), nullptr);

		const VK::Mesh mesh = VK::IntermediaryStorage::GetInstance().GetMesh(data);

		if (!mesh.valid) { continue; }

		bool result = false;
		std::uint32_t texID = VK::IntermediaryStorage::GetInstance().GetTextureID(data, m_commandPool, result);

		if (!result) { continue; }

		PushConstantData pcData
		{
			.world = data.transform,
			.time = Time::Get().GetRuntime(),
			.textureID = texID
		};

		vk::PushConstantsInfo pcInfo
		{
			.sType = vk::StructureType::ePushConstantsInfo,
			.layout = pipeline.GetLayout(),
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(PushConstantData),
			.pValues = &pcData,
		};

		commandBuffer.pushConstants2(pcInfo);
		commandBuffer.bindVertexBuffers(0, mesh.vertexBuffer, { 0 });
		commandBuffer.bindIndexBuffer(mesh.indexBuffer, 0, vk::IndexType::eUint32);
		commandBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
	}

	Debug::DebugDraw::Get().RenderToCamera(p_camera, commandBuffer, p_renderTarget.GetExtent(), m_swapchain.GetCurrentIndex());

	commandBuffer.endRendering();
	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR

	TransitionImageLayout(
		p_renderTarget.GetColorImage(),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::ImageAspectFlagBits::eColor,
		commandBuffer
	);

	//commandBuffer.end();
}

// Bind it to ImGui
void Renderer::RecordImGuiCommandBuffer(
	vk::raii::CommandBuffer &p_commandBuffer,
	uint32_t p_imageIndex)
{
	ZoneScoped;

	auto &commandBuffer = p_commandBuffer;
	TransitionImageLayout(
		m_swapchain.GetPresentImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor,
		commandBuffer
	);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

	vk::RenderingAttachmentInfo colorAttachmentInfo
	{
		.imageView = m_swapchain.GetPresentImageView(),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
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
		.pColorAttachments = &colorAttachmentInfo
	};


	commandBuffer.beginRendering(renderingInfo);	

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);

	commandBuffer.endRendering();

	TransitionImageLayout(
		m_swapchain.GetPresentImage(),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::ImageAspectFlagBits::eColor,
		commandBuffer
	);
}

//Need to create fences and semaphores for each frame in flight
//Creation of objects for parallellization, semaphores for GPU, fences for CPU
void Renderer::CreateSyncObjects()
{
	ZoneScoped;

	assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

	for (size_t i = m_swapchain.GetNumImages(); i > 0; i--)
	{
		m_renderFinishedSemaphores.emplace_back(m_context.GetDevice(), vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_presentCompleteSemaphores.emplace_back(m_context.GetDevice(), vk::SemaphoreCreateInfo());
		m_inFlightFences.emplace_back(m_context.GetDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
}

void Renderer::Draw(const Camera &p_camera, std::vector<RenderData> p_renderData)
{	
	ZoneScoped;

	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
	//while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = m_context.GetDevice().waitForFences(*m_inFlightFences[m_swapchain.GetCurrentIndex()], vk::True, std::numeric_limits<std::uint64_t>::max());
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}
	
	//vk::result, std::uint32_t
	//auto [result, imageIndex] = m_swapchain.Get().acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *m_presentCompleteSemaphores[m_swapchain.GetCurrentIndex()], nullptr);
	m_swapchain.AcquireNextImageIndex(m_context.GetDevice(), m_presentCompleteSemaphores.at(m_swapchain.GetCurrentIndex()), nullptr, std::numeric_limits<std::uint64_t>::max());
	std::uint32_t imageIndex = m_swapchain.GetCurrentIndex();
	
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	/*if (m_latestResult == vk::Result::eErrorOutOfDateKHR)
	{
		while ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&Event);
		}
		
		m_context.GetDevice().waitIdle();
		m_swapchain = { VK::IntermediaryStorage::GetInstance().GetAllocator(), m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window, m_context.GetSurface() };
		return;
	}*/

	VK::ViewProjectionBuffer vp
	{
		.view = p_camera.GetViewMatrix(),
		.proj = p_camera.GetProjectionMatrix()
	};

	m_vpBuffer.UpdateBuffer(vp);

	Debug::DebugDraw::Get().PrepareRender(m_swapchain.GetCurrentIndex());
	
	// Only reset the fence if we are submitting work
	m_context.GetDevice().resetFences(*m_inFlightFences[m_swapchain.GetCurrentIndex()]);

	ImGui::EndFrame();
	ImGui::Render();
	
	m_commandPool.GetBufferAt(m_swapchain.GetCurrentIndex()).reset();

	auto& commandBuffer = m_commandPool.GetBufferAt(m_swapchain.GetCurrentIndex());
	commandBuffer.begin({});

	RecordRenderTargetCommandBuffer(
		commandBuffer,
		VK::IntermediaryStorage::GetInstance().GetRenderTargetAt(0), // TODO: <REFACTOR/> 0 is a magic number and will not always be SceneViewWindows render targets if we make more render targets
		p_camera,
		p_renderData
	);

	RecordImGuiCommandBuffer(
		commandBuffer,
		imageIndex
	);

	Debug::DebugDraw::Get().ClearDrawCommands();

	commandBuffer.end();

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eAllCommands);
	
	const vk::SubmitInfo   submitInfo
	{ 
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*m_presentCompleteSemaphores[m_swapchain.GetCurrentIndex()],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*m_renderFinishedSemaphores[imageIndex]
	};
	
	m_context.GetQueue().submit(submitInfo, *m_inFlightFences[m_swapchain.GetCurrentIndex()]);

	const vk::PresentInfoKHR presentInfoKHR
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*m_renderFinishedSemaphores[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &*m_swapchain.Get(),
		.pImageIndices = &imageIndex
	};
	
	vk::Result result = m_context.GetQueue().presentKHR(presentInfoKHR);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
	{
		m_framebufferResized = false;
		while ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) != 0)
		{
			SDL_WaitEvent(&Event);
		}
		
		m_context.GetDevice().waitIdle();
		m_swapchain = { VK::IntermediaryStorage::GetInstance().GetAllocator(), m_context.GetDevice(), m_context.GetPhysicalDevice(), m_window, m_context.GetSurface() };
	}
	
	// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
	assert(result == vk::Result::eSuccess);
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
	p_initInfo.PipelineCache = nullptr;
	p_initInfo.DescriptorPool = *m_imGuiDescriptorPool;
	p_initInfo.MinImageCount = 2;
	p_initInfo.ImageCount = 2;
	p_initInfo.Allocator = nullptr;
	p_initInfo.PipelineInfoMain.RenderPass = nullptr;
	p_initInfo.PipelineInfoMain.Subpass = 0;
	p_initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	p_initInfo.CheckVkResultFn = CheckVkResult;

	p_initInfo.UseDynamicRendering = vk::True;

	m_imGuiColorFormat = static_cast<VkFormat>(m_swapchain.GetSurfaceFormat().format);
	VkFormat depthFormat = static_cast<VkFormat>(m_swapchain.GetDepthFormat());

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