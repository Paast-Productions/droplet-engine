#include "Graphics/VK/Pipeline.hpp"

#include <Graphics/VK/VertexBuffer.hpp>

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

Pipeline::Pipeline(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::ShaderModule &p_shaderModule, const PipelineConfig &p_pipelineConfig)
{
	assert(p_device != nullptr && p_shaderModule != nullptr);
	
	// TODO: Make dynamic
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex, 
		.module = p_shaderModule, 
		.pName = "vertMain"
	};
	
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eFragment, 
		.module = p_shaderModule, 
		.pName = "fragMain"
	};
	vk::PipelineShaderStageCreateInfo shaderStages[]
	{
		vertShaderStageInfo, 
		fragShaderStageInfo
	};

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
		.topology = p_pipelineConfig.Topology
	};
	
	// viewportCount and scissorCount must match.
	vk::PipelineViewportStateCreateInfo      viewportState
	{
		.viewportCount = p_pipelineConfig.ViewportCount,
		.scissorCount = p_pipelineConfig.ViewportCount
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer
	{ 
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = p_pipelineConfig.PolygonMode,
		.cullMode = p_pipelineConfig.CullMode,
		.frontFace = p_pipelineConfig.FrontFace,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f 
	};

	// Using multisampling is enabled or disabled via the pipeline config
	vk::PipelineMultisampleStateCreateInfo multisampling
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = p_pipelineConfig.UseMultisampling ? vk::True : vk::False
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
		.blendEnable = p_pipelineConfig.UseBlending ? vk::True : vk:: False,
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
		.dynamicStateCount = static_cast<uint32_t>(p_pipelineConfig.DynamicStates.size()),
		.pDynamicStates = p_pipelineConfig.DynamicStates.data()
	};

	m_pipelineLayout = vk::raii::PipelineLayout(p_device, p_pipelineConfig.PipelineLayoutInfo);

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
			 .stageCount = 2,
			 .pStages = shaderStages,
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
			.pColorAttachmentFormats = &p_pipelineConfig.SwapchainSurfaceFormat.format,
			.depthAttachmentFormat = depthFormat
		}
	};

	m_pipeline = vk::raii::Pipeline(p_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}
