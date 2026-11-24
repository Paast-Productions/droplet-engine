#include "TextRenderer.hpp"

#include "Camera.hpp"

namespace Droplet::Graphics::Vulkan
{
	namespace
	{
		struct CameraUniform
		{
			glm::mat4 view;
			glm::mat4 projection;
			float aspectRatio;
		};
		
		std::vector<char> ReadShader(const std::filesystem::path &p_path)
		{
			ZoneScoped;

			std::ifstream file(p_path, std::ios::binary | std::ios::ate);
			if (!file)
			{
				throw std::runtime_error("Failed to open debug draw shader: " + p_path.string());
			}

			const std::streamoff size = static_cast<std::streamoff>(file.tellg());
			if (size <= 0 || (size % sizeof(std::uint32_t)) != 0)
			{
				throw std::runtime_error("Invalid debug draw shader bytecode: " + p_path.string());
			}

			std::vector<char> code(static_cast<std::size_t>(size));

			file.seekg(0);
			file.read(code.data(), static_cast<std::streamsize>(code.size()));

			if (!file)
			{
				throw std::runtime_error("Failed to read debug draw shader: " + p_path.string());
			}

			return code;
		}

		vk::raii::ShaderModule CreateShaderModule(const vk::raii::Device &p_device, const std::filesystem::path &p_path)
		{
			ZoneScoped;

			const std::vector<char> code = ReadShader(p_path);

			const vk::ShaderModuleCreateInfo createInfo {
				.codeSize = code.size(),
				.pCode = reinterpret_cast<const std::uint32_t *>(code.data())
			};

			return vk::raii::ShaderModule(p_device, createInfo);
		}
		
		vma::raii::Buffer CreateHostBuffer(const vma::raii::Allocator &p_allocator, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage)
		{
			const vk::BufferCreateInfo bufferInfo {
				.size = p_size,
				.usage = p_usage,
				.sharingMode = vk::SharingMode::eExclusive
			};

			const vma::AllocationCreateInfo allocationInfo {
				.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
				.usage = vma::MemoryUsage::eAuto,
				.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};

			return vma::raii::Buffer(p_allocator, bufferInfo, allocationInfo);
		}
	}
}

void Droplet::Graphics::Vulkan::TextRenderer::Initialize(const Context &p_context,
	const vma::raii::Allocator &p_allocator, const CommandPool &p_commandPool, vk::Format p_colorFormat,
	vk::Format p_depthFormat, uint32_t p_framesInFlight)
{
	m_context = &p_context;
	m_allocator = &p_allocator;
	m_commandPool = &p_commandPool;
	m_frameDraws.resize(p_framesInFlight);
	
	const vk::raii::Device& device = p_context.GetDevice();
	
	const std::array<vk::DescriptorSetLayoutBinding, 4> bindings {{
		{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex 
		},
		{
			.binding = 1,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment
		},
		{	.binding = 2,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment
		},
		{
			.binding = 3,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
		}
	}};
	
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(device, vk::DescriptorSetLayoutCreateInfo {
		.bindingCount = bindings.size(),
		.pBindings = bindings.data()
	});
	
	const vk::PushConstantRange pushConstantRange {
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = sizeof(Text::TextPushConstants)
	};
	
	m_pipelineLayout = vk::raii::PipelineLayout(device, vk::PipelineLayoutCreateInfo {
		.setLayoutCount = 1,
		.pSetLayouts = &*m_descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	});
	
	const uint32_t maxDraws = p_framesInFlight * MAX_DRAWS_PER_FRAME;
	const std::array<vk::DescriptorPoolSize, 3> poolSizes{{
    			{
    				.type = vk::DescriptorType::eUniformBuffer,
    				.descriptorCount = 2
    			},
    			{
    				.type = vk::DescriptorType::eSampledImage,
    				.descriptorCount = 4
    			},
    			{
    				.type = vk::DescriptorType::eStorageBuffer,
    				.descriptorCount = 2
    			}
    		}};
	
	m_descriptorPool = vk::raii::DescriptorPool(device, vk::DescriptorPoolCreateInfo {
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = maxDraws,
		.poolSizeCount = poolSizes.size(),
		.pPoolSizes = poolSizes.data()
	});
	
	m_vertexShader = CreateShaderModule(device, "slug_test.vert.spv");
	m_fragmentShader = CreateShaderModule(device, "slug_test.frag.spv");
	
	const std::array<vk::PipelineShaderStageCreateInfo, 2> stages {{
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = *m_vertexShader,
				.pName = "slugVertMain"
			},
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = *m_fragmentShader,
				.pName = "slugFragMain"
			}
		}};
	
	const vk::PipelineVertexInputStateCreateInfo vertexInput{};
	
	const vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		.topology = vk::PrimitiveTopology::eTriangleList // TODO: Consider TriangleStrip to reduce vertices
	};
	
	const vk::PipelineViewportStateCreateInfo viewportState { 
		.viewportCount = 1, 
		.scissorCount = 1 
	};
	
	const vk::PipelineRasterizationStateCreateInfo rasterizer {
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.lineWidth = 1.0f
	};

	const vk::PipelineMultisampleStateCreateInfo multisampling { 
		.rasterizationSamples = vk::SampleCountFlagBits::e1
	};

	const vk::PipelineDepthStencilStateCreateInfo depthStencil {
		.depthTestEnable = vk::True,
		.depthWriteEnable = vk::False,
		.depthCompareOp = vk::CompareOp::eLessOrEqual
	};

	const vk::PipelineColorBlendAttachmentState blendAttachment {
		.blendEnable = vk::True,
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.alphaBlendOp = vk::BlendOp::eAdd,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	const vk::PipelineColorBlendStateCreateInfo blendState {
		.attachmentCount = 1,
		.pAttachments = &blendAttachment
	};
	
	const std::array<vk::DynamicState, 2> dynamicStates { 
		vk::DynamicState::eViewport, 
		vk::DynamicState::eScissor 
	};

	const vk::PipelineDynamicStateCreateInfo dynamicState {
		.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};
	
	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> createInfo {
		{
			.stageCount = static_cast<std::uint32_t>(stages.size()),
			.pStages = stages.data(),
			.pVertexInputState = &vertexInput,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &blendState,
			.pDynamicState = &dynamicState,
			.layout = *m_pipelineLayout
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &p_colorFormat,
			.depthAttachmentFormat = p_depthFormat
		}
	};
	m_pipeline = vk::raii::Pipeline(device, nullptr, createInfo.get<vk::GraphicsPipelineCreateInfo>());
	
}

void Droplet::Graphics::Vulkan::TextRenderer::BeginFrame(uint32_t p_frameIndex)
{
	m_frameDraws.at(p_frameIndex).clear();
}

void Droplet::Graphics::Vulkan::TextRenderer::EnsureFont(GUID p_fontGUID, const FontResource &p_font)
{
	if (m_fonts.contains(p_fontGUID))
		return;
	
	const std::vector<glm::vec4>& curveData = p_font.GetCurvesTextureData();
	const std::vector<glm::u16vec2>& bandData = p_font.GetBandsTextureData();
	constexpr uint32_t width = FontResource::TEXTURE_WIDTH;
	
	if (curveData.empty() || bandData.empty() ||
		curveData.size() % width != 0 || bandData.size() % width != 0)
	{
		throw std::runtime_error("Invalid padded .slug texture data.");
	}
	
	const uint32_t curveHeight = static_cast<uint32_t>(curveData.size() / width);
	const uint32_t bandHeight = static_cast<uint32_t>(bandData.size() / width);
	
	GPUFont gpu{};
	gpu.curves.emplace(
		*m_allocator, *m_commandPool, *m_context,
		reinterpret_cast<const std::byte*>(curveData.data()), curveData.size() * sizeof(glm::vec4),
		width, curveHeight, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	gpu.bands.emplace(
		*m_allocator, *m_commandPool, *m_context,
		reinterpret_cast<const std::byte*>(bandData.data()), bandData.size() * sizeof(glm::u16vec2),
		width, bandHeight, vk::Format::eR16G16Uint, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	gpu.curvesView.emplace(
		m_context->GetDevice(), gpu.curves->Get(), vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor);
	gpu.bandsView.emplace(
		m_context->GetDevice(), gpu.bands->Get(), vk::Format::eR16G16Uint, vk::ImageAspectFlagBits::eColor);
	
	m_fonts.emplace(p_fontGUID, std::move(gpu));
}

uint32_t Droplet::Graphics::Vulkan::TextRenderer::Prepare(uint32_t p_frameIndex, GUID p_fontGUID,
	const Camera &p_camera, const std::vector<Text::GlyphInstance> &p_instances, const glm::mat4 &p_model,
	const glm::vec4 &p_color)
{
	if (p_instances.empty())
		throw std::runtime_error("Cannot prepare an empty text draw.");
	
	auto& draws = m_frameDraws.at(p_frameIndex);
	if (draws.size() >= MAX_DRAWS_PER_FRAME)
		throw std::runtime_error("Too many text draws for current frame.");
	
	const auto fontIt = m_fonts.find(p_fontGUID);
	if (fontIt == m_fonts.end())
		throw std::runtime_error("Font GPU data not uploaded, call EnsureFont first.");
	
	const GPUFont& font = fontIt->second;
	const vk::raii::Device& device = m_context->GetDevice();
	
	auto draw = std::make_unique<PrepareDraw>();
	draw->glyphBuffer = StorageBuffer(*m_allocator, p_instances);
	draw->cameraBuffer = CreateHostBuffer(*m_allocator, sizeof(CameraUniform), vk::BufferUsageFlagBits::eUniformBuffer);
	draw->model = p_model;
	draw->color = p_color;
	draw->instanceCount = p_instances.size();
	
	const CameraUniform uniform {
		p_camera.GetViewMatrix(),
		p_camera.GetProjectionMatrix(),
		p_camera.GetAspect()
	};
	draw->cameraBuffer.getAllocation().copyFromMemory(&uniform, 0, sizeof(uniform));
	
	const vk::DescriptorSetAllocateInfo allocInfo {
		.descriptorPool = *m_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &*m_descriptorSetLayout
	};
	draw->descriptorSet = std::move(device.allocateDescriptorSets(allocInfo).front());
	
	const vk::DescriptorBufferInfo cameraBufferInfo{
		.buffer = draw->cameraBuffer,
		.offset = 0,
		.range = sizeof(CameraUniform)
	};
	
	const vk::DescriptorImageInfo curveInfo{
		.sampler = nullptr,
		.imageView = font.curvesView->Get(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};
	
	const vk::DescriptorImageInfo bandInfo {
		.sampler = nullptr,
		.imageView = font.bandsView->Get(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};
	
	const vk::DescriptorBufferInfo glyphBufferInfo {
		.buffer = draw->glyphBuffer.Get(),
		.offset = 0,
		.range = vk::WholeSize
	};
	
	const std::array<vk::WriteDescriptorSet, 4> writes {{
		{
			.dstSet = *draw->descriptorSet,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &cameraBufferInfo
		},
		{
			.dstSet = *draw->descriptorSet,
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.pImageInfo = &curveInfo
		},
		{
			.dstSet = *draw->descriptorSet,
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.pImageInfo = &bandInfo
		},
		{
			.dstSet = *draw->descriptorSet,
			.dstBinding = 3,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &glyphBufferInfo
		}
	}};
	
	device.updateDescriptorSets(writes, {});
	
	const uint32_t index = draws.size();
	draws.push_back(std::move(draw));
	return index;
}

void Droplet::Graphics::Vulkan::TextRenderer::Record(const vk::raii::CommandBuffer &p_commandBuffer,
	uint32_t p_frameIndex, uint32_t p_drawIndex, vk::Extent2D p_extent) const
{
	if (p_extent.width == 0 || p_extent.height == 0)
		return;
	
	const PrepareDraw& draw = *m_frameDraws.at(p_frameIndex).at(p_drawIndex);
	const Text::TextPushConstants push {
		.model = draw.model,
		.color = draw.color
	};
	
	const vk::Viewport viewport {
		.x = 0.0f,
		.y = static_cast<float>(p_extent.height),
		.width = static_cast<float>(p_extent.width),
		.height = -static_cast<float>(p_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	
	const vk::Rect2D scissor{ 
		vk::Offset2D(0, 0), 
		p_extent 
	};
	
	p_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
	p_commandBuffer.setViewport(0, viewport);
	p_commandBuffer.setScissor(0, scissor);
	p_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, *draw.descriptorSet, {});
	p_commandBuffer.pushConstants(
		*m_pipelineLayout,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		0,
		sizeof(push),
		&push
	);
	p_commandBuffer.draw(6, draw.instanceCount, 0,0);
}

void Droplet::Graphics::Vulkan::TextRenderer::Shutdown()
{
	m_frameDraws.clear();
	m_fonts.clear();
	m_pipeline = nullptr;
	m_fragmentShader = nullptr;
	m_vertexShader = nullptr;
	m_descriptorPool = nullptr;
	m_pipelineLayout = nullptr;
	m_descriptorSetLayout = nullptr;
	m_context = nullptr;
	m_allocator = nullptr;
	m_commandPool = nullptr;
}
