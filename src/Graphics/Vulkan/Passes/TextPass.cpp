#include "TextPass.hpp"

Droplet::Common::ShaderDesc Droplet::Graphics::Vulkan::TextPass::ShaderAsset()
{
	return Common::ShaderDesc{
		.moduleName = "text",
		.sourcePath = ROOT_PATH "/Assets/Shader/text.slang",
		.vertexEntry = "vMain",
		.fragmentEntry = "fMain"
	};
}

Droplet::Graphics::Vulkan::TextPass::TextPass(const Context &p_context, RenderDevice &p_device,
	uint32_t p_framesInFlight)
		: m_context(&p_context), m_gpu(&p_device.Resources()), m_textures(&p_device.Textures())
{
	const vk::DescriptorSetLayoutBinding viewBinding {
		.binding = 0,
		.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex
	};
	
	m_viewLayout = vk::raii::DescriptorSetLayout(
		m_context->GetDevice(),
		vk::DescriptorSetLayoutCreateInfo{
			.bindingCount = 1,
			.pBindings = &viewBinding
		}
	);
	
	const vk::DescriptorSetLayoutBinding glyphBinding{
		.binding = 0,
		.descriptorType = vk::DescriptorType::eStorageBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
	};
	
	m_glyphLayout = vk::raii::DescriptorSetLayout(
		m_context->GetDevice(),
		vk::DescriptorSetLayoutCreateInfo{
			.bindingCount = 1,
			.pBindings = &glyphBinding
		}
	);
	
	const std::array layouts{
		*m_viewLayout,
		*m_glyphLayout,
		m_textures->Layout()
	};
	
	const vk::PushConstantRange pushRange{
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = sizeof(Text::TextPushConstants)
	};
	
	m_pipelineLayout = vk::raii::PipelineLayout(
		m_context->GetDevice(),
		vk::PipelineLayoutCreateInfo{
			.setLayoutCount = layouts.size(),
			.pSetLayouts = layouts.data(),
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushRange
		}
	);
	
	const std::array poolSizes {
		vk::DescriptorPoolSize{
			vk::DescriptorType::eUniformBufferDynamic,
			p_framesInFlight
		},
		vk::DescriptorPoolSize{
			vk::DescriptorType::eStorageBuffer,
			p_framesInFlight
		}
	};
	
	m_descriptorPool = vk::raii::DescriptorPool(
		m_context->GetDevice(),
		vk::DescriptorPoolCreateInfo{
			.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			.maxSets = p_framesInFlight * 2,
			.poolSizeCount = poolSizes.size(),
			.pPoolSizes = poolSizes.data()
		}
	);
	
	const std::vector<vk::DescriptorSetLayout> viewLayouts(p_framesInFlight, *m_viewLayout);
	const std::vector<vk::DescriptorSetLayout> glyphLayouts(p_framesInFlight, *m_glyphLayout);
	
	m_viewSets = vk::raii::DescriptorSets(
		m_context->GetDevice(),
		vk::DescriptorSetAllocateInfo{
			.descriptorPool = *m_descriptorPool,
			.descriptorSetCount = p_framesInFlight,
			.pSetLayouts = viewLayouts.data()
		}
	);
	
	m_glyphSets = vk::raii::DescriptorSets(
		m_context->GetDevice(),
		vk::DescriptorSetAllocateInfo{
			.descriptorPool = *m_descriptorPool,
			.descriptorSetCount = p_framesInFlight,
			.pSetLayouts = glyphLayouts.data()
		}
	);
	
	m_viewBuffers.resize(p_framesInFlight);
	m_glyphBuffers.resize(p_framesInFlight);
	
	const vk::DeviceSize alignment = m_context->GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
	
	m_viewStride = ((sizeof(Common::ViewProjectionBuffer) + alignment - 1) / alignment) * alignment;
	
	if (!p_device.Shaders().GetOrCreate(SHADER_ID, ShaderAsset()))
	{
		throw std::runtime_error("Failed to compile the built-in slug text shader.");
	}
}

Droplet::Graphics::Vulkan::GraphicsPipelineDesc Droplet::Graphics::Vulkan::TextPass::MakePipelineDesc(
	vk::Format p_colorFormat, vk::Format p_depthFormat) const
{
	GraphicsPipelineDesc desc{};
	
	desc.vertexShader = SHADER_ID;
	desc.fragmentShader = SHADER_ID;
	desc.hasFragmentShader = true;
	
	desc.layout = *m_pipelineLayout;
	
	desc.vertexLayout = {}; // No vertices are being passed
	
	desc.topology = vk::PrimitiveTopology::eTriangleList;
	
	desc.raster.cullMode = vk::CullModeFlagBits::eNone;
	desc.raster.polygonMode = vk::PolygonMode::eFill;
	
	desc.depth.testEnable = p_depthFormat != vk::Format::eUndefined;
	desc.depth.writeEnable = false;
	desc.depth.compareOp = vk::CompareOp::eLessOrEqual;
	
	desc.colorFormats = {p_colorFormat};
	desc.colorBlend = {BlendMode::AlphaBlend};
	desc.depthFormat = p_depthFormat;
	
	return desc;
}

void Droplet::Graphics::Vulkan::TextPass::BeginFrame(uint32_t p_frameSlot, const Render::PreparedFrame &p_frame)
{
	m_frameSlot = p_frameSlot;
	
	const vk::DeviceSize viewBytes = std::max<size_t>(p_frame.views.size(), 1) * m_viewStride;
	
	Buffer& viewBuffer = m_viewBuffers[p_frameSlot];
	
	if (!viewBuffer || viewBuffer.GetSize() < viewBytes)
	{
		viewBuffer = Buffer::Create(
			m_context->GetAllocator(),
			BufferCreateOptions::Uniform(
				std::max<vk::DeviceSize>(viewBytes * 2, m_viewStride * 8)
			)
		);
		
		const vk::DescriptorBufferInfo info{
			.buffer = viewBuffer.Get(),
			.offset = 0,
			.range = sizeof(Common::ViewProjectionBuffer) 
		};
		
		m_context->GetDevice().updateDescriptorSets(
			vk::WriteDescriptorSet{
				.dstSet = *m_viewSets[p_frameSlot],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
				.pBufferInfo = &info,
			},
			{}
		);
	}
	
	const vk::DeviceSize glyphBytes = p_frame.glyphInstances.size() * sizeof(Text::GlyphInstance);
	
	if (glyphBytes == 0)
		return;
	
	Buffer& glyphBuffer = m_glyphBuffers[p_frameSlot];
	
	if (!glyphBuffer || glyphBuffer.GetSize() < glyphBytes)
	{
		glyphBuffer = Buffer::Create(
			m_context->GetAllocator(),
			BufferCreateOptions::For(
				std::max<vk::DeviceSize>(glyphBytes * 2, 64 * 1024),
				vk::BufferUsageFlagBits::eStorageBuffer,
				MemoryAccess::Dynamic
			)
		);
		
		const vk::DescriptorBufferInfo info{
			.buffer = glyphBuffer.Get(),
			.offset = 0,
			.range = glyphBuffer.GetSize()
		};
		
		m_context->GetDevice().updateDescriptorSets(
			vk::WriteDescriptorSet{
				.dstSet = *m_glyphSets[p_frameSlot],
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = &info
			},
			{}
		);
	}
	
	for (std::size_t viewIndex = 0; viewIndex < p_frame.views.size(); ++viewIndex)
	{
		const Render::DrawList& view = p_frame.views[viewIndex];
		const Common::ViewProjectionBuffer uniforms{ .view = view.view, .proj = view.proj };
		viewBuffer.Write(&uniforms, sizeof(uniforms), viewIndex * m_viewStride);
	}
	
	glyphBuffer.Write(p_frame.glyphInstances.data(), glyphBytes);
}

void Droplet::Graphics::Vulkan::TextPass::Record(vk::CommandBuffer p_cmd, vk::Extent2D p_extent,
	const Render::DrawList &p_view, uint32_t p_viewIndex, const PipelineLibrary &p_pipelines)
{
	if (p_view.textItems.empty())
		return;
	
	const vk::Viewport viewport{
		.x = 0.0f,
		.y = static_cast<float>(p_extent.height),
		.width = static_cast<float>(p_extent.width),
		.height = -static_cast<float>(p_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f 
	};
	
	const vk::Rect2D scissor{
		.offset = {0, 0},
		.extent = p_extent
	};
	
	p_cmd.setViewport(0, viewport);
	p_cmd.setScissor(0, scissor);
	
	const uint32_t viewOffset = static_cast<uint32_t>(p_viewIndex * m_viewStride);
	
	p_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, *m_viewSets[m_frameSlot], viewOffset);
	p_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 1, *m_glyphSets[m_frameSlot], {});
	p_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 2, m_textures->Set(m_frameSlot), {});
	
	for (const Render::TextDrawItem& item : p_view.textItems)
	{
		const GraphicsPipeline* pipeline = p_pipelines.Find(item.pipeline);
		const GpuTexture* fontTexture = m_gpu->Find(item.fontTexture);
		
		if (!pipeline || !fontTexture || item.instanceCount == 0)
			continue;
		
		pipeline->Bind(p_cmd);
		
		const Text::TextPushConstants constants{
			.model = item.model,
			.color = item.color,
			.textureId = fontTexture->slot.Index(),
			.firstInstance = item.firstInstance
		};
		
		p_cmd.pushConstants(
			*m_pipelineLayout,
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			0,
			sizeof(constants),
			&constants
		);
		
		p_cmd.draw(6, item.instanceCount, 0, 0);
	}
}
