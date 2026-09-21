#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <string>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <optional>

namespace Droplet::Graphics::VK
{
	struct PipelineConfig
	{
		bool UseMultisampling = false;
		bool UseBlending = false;
		uint32_t ViewportCount = 1;
		vk::PolygonMode PolygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlagBits CullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace FrontFace = vk::FrontFace::eClockwise;
		vk::PrimitiveTopology Topology = vk::PrimitiveTopology::eTriangleList;
		
		std::vector<vk::DynamicState> DynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineLayoutCreateInfo PipelineLayoutInfo = {
			.setLayoutCount = 0,
			.pushConstantRangeCount = 0
		};	
		
		vk::SurfaceFormatKHR SwapchainSurfaceFormat;
	};

	class Pipeline
	{
	public:
		Pipeline() = delete;
		Pipeline(const vk::raii::Device &p_device, const vk::raii::ShaderModule &p_shaderModule, const PipelineConfig &p_pipelineConfig);
		
		~Pipeline() = default;
		
		[[nodiscard]] vk::raii::Pipeline& Get();

	private:
		vk::raii::PipelineLayout m_pipelineLayout = nullptr;
		vk::raii::Pipeline m_pipeline = nullptr;
	};
}