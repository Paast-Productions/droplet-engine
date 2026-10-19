#include "Graphics/VK/Pipeline.hpp"

#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/RenderData.hpp>

using namespace Droplet::Graphics::VK;

vk::Format FindSupportedFormat(const vk::raii::PhysicalDevice &p_physicalDevice, const std::vector<vk::Format> &p_candidates, vk::ImageTiling p_tiling, vk::FormatFeatureFlags p_features)
{
	for (const auto format : p_candidates) {
		vk::FormatProperties props = p_physicalDevice.getFormatProperties(format);

		if (((p_tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & p_features) == p_features)) ||
			((p_tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & p_features) == p_features)))
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

Pipeline::Pipeline(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::DescriptorSetLayout &p_descriptorSetLayout, std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> &shaderModules, vk::SurfaceFormatKHR p_surfaceFormat)
{
	vk::PushConstantRange pushConstantRange
	{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = sizeof(PushConstantData)
	};
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts 
	{
		p_descriptorSetLayout
	};
	
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size()),
		.pSetLayouts = descriptorSetLayouts.data(), 
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};	
		
	m_pipelineLayout = vk::raii::PipelineLayout(p_device, pipelineLayoutInfo);
	
	struct PipelineConfig
	{
		bool UseMultisampling = false;
		bool UseBlending = false;
		std::uint32_t ViewportCount = 1;
		vk::PolygonMode PolygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlagBits CullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace FrontFace = vk::FrontFace::eCounterClockwise;
		vk::PrimitiveTopology Topology = vk::PrimitiveTopology::eTriangleList;
		
		std::vector<vk::DynamicState> DynamicStates
		{
			vk::DynamicState::eViewport, 
			vk::DynamicState::eScissor
		};
		
		vk::SurfaceFormatKHR SwapchainSurfaceFormat {};
	} pipelineConfig;
	
	vk::PipelineCacheCreateInfo cacheInfo {};
	vk::raii::PipelineCache pipelineCache(p_device, cacheInfo);
	
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos {};
	for (const auto &pair : shaderModules)
	{
		vk::PipelineShaderStageCreateInfo stageInfo 
		{
			.stage = pair.second, 
			.module = pair.first, 
			.pName = "main"
		};
		
		shaderStageInfos.push_back(stageInfo);
	}
	
	//--Added for basic model rendering functionality
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo   vertexInputInfo
	{ 
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data() 
	};
	//--
	
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.topology = pipelineConfig.Topology
	};
	
	// viewportCount and scissorCount must match.
	vk::PipelineViewportStateCreateInfo      viewportState
	{
		.viewportCount = pipelineConfig.ViewportCount,
		.scissorCount = pipelineConfig.ViewportCount
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer
	{ 
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = pipelineConfig.PolygonMode,
		.cullMode = pipelineConfig.CullMode,
		.frontFace = pipelineConfig.FrontFace,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f 
	};

	// Using multisampling is enabled or disabled via the pipeline config
	vk::PipelineMultisampleStateCreateInfo multisampling
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = pipelineConfig.UseMultisampling ? vk::True : vk::False
	};

	//--Added for basic model rendering functionality
	vk::PipelineDepthStencilStateCreateInfo depthStencil
	{
		.depthTestEnable = vk::True,
		.depthWriteEnable = vk::True,
		.depthCompareOp = vk::CompareOp::eLess,
		.depthBoundsTestEnable = vk::False,
		.stencilTestEnable = vk::False 
	};
	//--

	// Blending is enabled or disabled via the pipeline config
	vk::PipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = pipelineConfig.UseBlending ? vk::True : vk:: False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA 
	};
	
	vk::PipelineColorBlendStateCreateInfo colorBlending 
	{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	// Dynamic states are defined in the pipeline config
	vk::PipelineDynamicStateCreateInfo dynamicState 
	{
		.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.DynamicStates.size()),
		.pDynamicStates = pipelineConfig.DynamicStates.data()
	};
	
	vk::Format depthFormat = FindSupportedFormat(
		p_physicalDevice,
		{ 
			vk::Format::eD32Sfloat, 
			vk::Format::eD32SfloatS8Uint, 
			vk::Format::eD24UnormS8Uint 
		},
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
	
	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain
	{
		{
			 .stageCount = static_cast<std::uint32_t>(shaderStageInfos.size()),
			 .pStages = shaderStageInfos.data(),
			 .pVertexInputState = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState = &multisampling,
			 .pDepthStencilState = &depthStencil,
			 .pColorBlendState = &colorBlending,
			 .pDynamicState = &dynamicState,
			 .layout = m_pipelineLayout,
			 .renderPass = nullptr
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &p_surfaceFormat.format,
			.depthAttachmentFormat = depthFormat
		}
	};

	m_pipeline = vk::raii::Pipeline(p_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}
