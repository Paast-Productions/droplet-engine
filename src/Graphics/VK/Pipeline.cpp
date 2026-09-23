#include "Graphics/VK/Pipeline.hpp"

using namespace Droplet::Graphics::VK;

Pipeline::Pipeline(const vk::raii::Device &p_device, const vk::raii::ShaderModule &p_shaderModule, const PipelineConfig &p_pipelineConfig)
{
	assert(p_device != nullptr && p_shaderModule != nullptr);
	
	// This should probably be dynamic somehow
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = p_shaderModule, .pName = "vertMain" };
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = p_shaderModule, .pName = "fragMain" };
	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = p_pipelineConfig.Topology };
	
	// Viewportcount and Scissorcount must match.
	vk::PipelineViewportStateCreateInfo      viewportState{ .viewportCount = p_pipelineConfig.ViewportCount, .scissorCount = p_pipelineConfig.ViewportCount };

	vk::PipelineRasterizationStateCreateInfo rasterizer{ .depthClampEnable = vk::False,
														.rasterizerDiscardEnable = vk::False,
														.polygonMode = p_pipelineConfig.PolygonMode,
														.cullMode = p_pipelineConfig.CullMode,
														.frontFace = p_pipelineConfig.FrontFace,
														.depthBiasEnable = vk::False,
														.lineWidth = 1.0f };

	// Using multisampling is enabled or disabled via the pipeline config
	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = p_pipelineConfig.UseMultisampling ? vk::True : vk::False
	};

	// Blending is enabled or disabled via the pipeline config
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = p_pipelineConfig.UseBlending ? vk::True : vk:: False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };
	vk::PipelineColorBlendStateCreateInfo colorBlending {
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	// Dynamic states are defined in the pipeline config
	vk::PipelineDynamicStateCreateInfo dynamicState {
		.dynamicStateCount = static_cast<uint32_t>(p_pipelineConfig.DynamicStates.size()),
		.pDynamicStates = p_pipelineConfig.DynamicStates.data()
	};

	m_pipelineLayout = vk::raii::PipelineLayout(p_device, p_pipelineConfig.PipelineLayoutInfo);

	vk::PipelineVertexInputStateCreateInfo   vertexInputInfo;
	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
		{
			 .stageCount = 2,
			 .pStages = shaderStages,
			 .pVertexInputState = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState = &multisampling,
			 .pColorBlendState = &colorBlending,
			 .pDynamicState = &dynamicState,
			 .layout = m_pipelineLayout,
			 .renderPass = nullptr
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &p_pipelineConfig.SwapchainSurfaceFormat.format
		} 
	};

	m_pipeline = vk::raii::Pipeline(p_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

[[nodiscard]] vk::raii::Pipeline& Pipeline::Get()
{
	return m_pipeline;
}
