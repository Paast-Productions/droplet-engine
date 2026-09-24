#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <slang/slang.h>

namespace Droplet::Graphics::VK
{
	/// @struct PipelineConfig 
	/// @brief Pipeline Creation Configuration
	/// Options:
	/// - bool								UseMultisampling [FALSE]
	/// - bool								UseBlending [FALSE]
	/// - std::uint32_t						ViewportCount [1]
	/// - vk::PolygonMode					PolygonMode [vk::PolygonMode::eFill]
	/// - vk::CullModeFlagBits				CullMode [vk::CullModeFlagBits::eBack]
	/// - vk::FrontFace						FrontFace [vk::FrontFace::eClockwise]
	/// - vk::PrimitiveTopology				Topology [vk::PrimitiveTopology::eTriangleList]
	/// - std::vector<vk::DynamicState>		DynamicStates [{vk::DynamicState::eViewport, vk::DynamicState::eScissor}]
	/// - vk::PipelineLayoutCreateInfo		PipelineLayoutInfo [{.setLayoutCount = 0, .pushConstantRangeCount = 0}]
	/// - vk::SurfaceFormatKHR				SwapchainSurfaceFormat [{}]
	
	struct PipelineConfig
	{
		bool UseMultisampling = false;
		bool UseBlending = false;
		std::uint32_t ViewportCount = 1;
		vk::PolygonMode PolygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlagBits CullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace FrontFace = vk::FrontFace::eClockwise;
		vk::PrimitiveTopology Topology = vk::PrimitiveTopology::eTriangleList;
		
		std::vector<vk::DynamicState> DynamicStates
		{
			vk::DynamicState::eViewport, 
			vk::DynamicState::eScissor
		};
		
		vk::PipelineLayoutCreateInfo PipelineLayoutInfo
		{
			.setLayoutCount = 0,
			.pushConstantRangeCount = 0
		};	
		
		vk::SurfaceFormatKHR SwapchainSurfaceFormat {};
	};

	/// @class Pipeline
	/// @brief Vulkan Pipeline
	class Pipeline
	{
	public:
		Pipeline() = delete;
		
		// TODO: refactor vk::raii:ShaderModule to accept multiple ShaderModules
		
		/// @brief Pipeline constructor
		/// @param p_device Vulkan Device
		/// @param p_physicalDevice Hardware Device
		/// @param p_shaderModule Vulkan Shader Module used for Pipeline creation 
		/// @param p_pipelineConfig Pipeline Configuration Struct
		Pipeline(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::ShaderModule &p_shaderModule, const PipelineConfig &p_pipelineConfig);
		
		~Pipeline() = default;
		
		/// @brief Getter-function for a vulkan pipeline
		/// @returns Vulkan Pipeline 
		[[nodiscard]] const vk::raii::Pipeline &Get();

	private:
		vk::raii::PipelineLayout m_pipelineLayout = nullptr;
		vk::raii::Pipeline m_pipeline = nullptr;
	};

	inline const vk::raii::Pipeline &Pipeline::Get()
	{
		return m_pipeline;
	}
}
