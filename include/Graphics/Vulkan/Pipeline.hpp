#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
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

	/// @class Pipeline
	/// @brief Vulkan Pipeline
	class Pipeline
	{
	public:
		// TODO: (RENDERING) Pipeline() = delete?
		// Author: SkyH1ghDev
		Pipeline() :
			m_pipelineLayout { nullptr },
			m_pipeline{ nullptr } {}
	
		/// @brief Pipeline Nullptr Constructor
		Pipeline(nullptr_t p_nullptr) :
			m_pipelineLayout { p_nullptr },
			m_pipeline { p_nullptr } {}
		
		/// @brief Pipeline constructor
		/// @param p_device Vulkan Device
		/// @param p_descriptorSetLayouts Vulkan Descriptor Set Layouts
		/// @param p_physicalDevice Hardware Device
		/// @param shaderModules Queue of Vulkan Shader Modules and their shader type
		/// @param p_surfaceFormat Format of the render Surface
		/// @param p_transparent
		Pipeline(const vk::raii::Device &p_device,
				const vk::raii::PhysicalDevice &p_physicalDevice,
				const std::vector<vk::DescriptorSetLayout> &p_descriptorSetLayouts,
				std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> &shaderModules,
				vk::SurfaceFormatKHR p_surfaceFormat, bool p_transparent);

		~Pipeline() = default;

		Pipeline(const Pipeline &p_other) = delete;
		Pipeline &operator=(const Pipeline &p_other) = delete;
		
		Pipeline(Pipeline &&p_other) noexcept
		{
			std::swap(m_pipeline, p_other.m_pipeline);
			std::swap(m_pipelineLayout, p_other.m_pipelineLayout);
		}
		
		Pipeline &operator=(Pipeline &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;	
			}
			
			std::swap(m_pipeline, p_other.m_pipeline);
			std::swap(m_pipelineLayout, p_other.m_pipelineLayout);
			
			return *this;
		} 
		
		bool operator==(const Pipeline &p_other) const
		{
			return (m_pipeline == p_other.m_pipeline				&&
					m_pipelineLayout == p_other.m_pipelineLayout);
		}
		
		/// @brief Getter-function for a Vulkan pipeline
		/// @returns Vulkan Pipeline 
		[[nodiscard]] const vk::raii::Pipeline &Get() const { return m_pipeline; }
		
		/// @brief Getter-function for a Vulkan Pipeline Layout
		/// @returns Vulkan Pipeline Layout
		[[nodiscard]] const vk::raii::PipelineLayout &GetLayout() const { return m_pipelineLayout; }

	private:
		vk::raii::PipelineLayout m_pipelineLayout { nullptr };
		vk::raii::Pipeline m_pipeline { nullptr };
	};
}
