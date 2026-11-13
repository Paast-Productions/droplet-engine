#include "SlugPrototype.hpp"
#include <Graphics/VK/Camera.hpp>
#include <fstream>
#include <Graphics/VK/Image.hpp>

// Shamelessly stolen from DebugDraw.cpp
std::vector<char> Droplet::Graphics::VK::Slug::ReadShader(const std::filesystem::path &p_path)
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

vk::raii::ShaderModule Droplet::Graphics::VK::Slug::CreateShaderModule(const vk::raii::Device &p_device, const std::filesystem::path &p_path)
{
	ZoneScoped;

	const std::vector<char> code = ReadShader(p_path);

	const vk::ShaderModuleCreateInfo createInfo {
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const std::uint32_t *>(code.data())
	};

	return vk::raii::ShaderModule(p_device, createInfo);
}

vma::raii::Buffer Droplet::Graphics::VK::Slug::CreateHostBuffer(const vma::raii::Allocator &p_allocator,
	vk::DeviceSize p_size, vk::BufferUsageFlags p_usage)
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

vma::raii::Buffer CreateHostBuffer(const vma::raii::Allocator &p_allocator, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage)
{
	ZoneScoped;

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

void Droplet::Graphics::VK::Slug::SlugPrototype::Initialize(const Context &p_context, const vma::raii::Allocator &p_allocator, const vk::raii::DescriptorPool &p_descriptorPool, vk::Format p_colorFormat,
	vk::Format p_depthFormat, const CommandPool &p_commandPool)
{
	m_device = &p_context.GetDevice();
	m_descriptorPool = &p_descriptorPool;
	m_colorFormat = p_colorFormat;
	m_depthFormat = p_depthFormat;
	m_allocator = &p_allocator;
	
	// Glyph rendering
	if (!LoadFont(R"(C:\Users\linkl\Desktop\dev\droplet-engine\Assets\Fonts\JetBrainsMono-Regular.slug)"))
	{
		return;
	}
	
	m_curveTexture.emplace(
			p_allocator,
			p_commandPool,
			p_context,
			reinterpret_cast<const std::byte*>(m_curvesTextureData.data()),
			m_curvesTextureData.size() * sizeof(glm::vec4),
			m_curveTextureWidth,
			m_curveTextureHeight,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
	
	m_bandTexture.emplace(
		p_allocator,
		p_commandPool,
		p_context,
		reinterpret_cast<const std::byte *>(m_bandsTextureData.data()),
		m_bandsTextureData.size() * sizeof(glm::u16vec2),
		m_bandsTextureWidth,
		m_bandsTextureHeight,
		vk::Format::eR16G16Uint,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
		);
	
	m_curveTextureView.emplace(p_context.GetDevice(),m_curveTexture->Get(), vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor);
	m_bandTextureView.emplace(p_context.GetDevice(),m_bandTexture->Get(), vk::Format::eR16G16Uint, vk::ImageAspectFlagBits::eColor);
	

	// Alternative way of calculating text size. This is a bit weird, though, since font size in pt traditionally uses
	// DPI and does not refer to a "unit" size. Keeping this here for future reference.
	//
	// constexpr float referenceFontSizePt = 24.0f;
	// constexpr float referenceEmSize = 0.5f;
	//
	// float fontSizePt = 48.0f;
	// float emSize = (fontSizePt / referenceFontSizePt) * referenceEmSize;
	
	constexpr float emSize = 0.5f; // em-to-world conversion (1 em = 0.5 units)
	float fontScale = emSize / static_cast<float>(m_fontHeader.unitsPerEm);
	
	const float lineHeight = static_cast<float>(m_fontHeader.ascent - m_fontHeader.descent + m_fontHeader.lineGap) * fontScale;

	glm::vec4 textColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	glm::vec2 pen{0.0f};
	int i = 0;
	m_glyphInstances.clear();
	m_glyphInstances.reserve((m_text.size()));
	for (uint32_t codePoint : DecodeUTF8(m_text))
	{
		if (codePoint == U'\n')
		{
			pen.x = 0.0f;
			pen.y -= lineHeight;
			continue;
		}
		
		const SlugCodePoint* glyph = GetGlyph(codePoint);
		
		float advance = 0.0f;
		bool shouldRender = false;
		GlyphInstance instance{};
		
		if (!glyph)
		{
			advance = emSize * 0.6f;
			const float width = emSize * 0.5f;
			const float height = emSize * 0.7f;
			
			instance = {
				.scaleBias = {
					width,
					height,
					pen.x + advance * 0.5f,
					pen.y + height * 0.5f,
				},
				.glyphBandScale = glm::vec4(0.0f),
				.glyphColor = textColor,
				.bandMaxTexCoords = glm::uvec4(0)
			};
			shouldRender = true; // We want to render a "missing char" glyph
		}
		else if (glyph->bandCount > 0 && glyph->bandDimX && glyph->bandDimY)
		{
			const float width = glyph->width;
			const float height = glyph->height;
			
			const float quadWidth = width * fontScale;
			const float quadHeight = height * fontScale;
			
			const float glyphX = pen.x + static_cast<float>(glyph->xMin) * fontScale;
			const float glyphY = pen.y + static_cast<float>(glyph->yMin) * fontScale;
			
			instance = {
				.scaleBias = {
					quadWidth,
					quadHeight,
					glyphX + quadWidth * 0.5f,
					glyphY + quadHeight * 0.5f
				},
				.glyphBandScale = {
					width,
					height,
					width / glyph->bandDimX,
					height / glyph->bandDimY
				},
				.glyphColor = textColor,
				.bandMaxTexCoords = {
					glyph->bandCount - 1,
					glyph->bandCount - 1,
					glyph->bandsTexCoordX,
					glyph->bandsTexCoordY
				}
			};
			
			shouldRender = true;
		}
		
		if (glyph)
			advance = static_cast<float>(glyph->advanceWidth) * fontScale;
		
		pen.x += advance;
		
		if (shouldRender)
			m_glyphInstances.push_back(instance);
	}
	
	m_storageBuffer = StorageBuffer(p_allocator, m_glyphInstances);
	
	
	
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
				.stageFlags = vk::ShaderStageFlagBits::eAllGraphics
			}
		}};
	

	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(p_context.GetDevice(), vk::DescriptorSetLayoutCreateInfo {
		.bindingCount = bindings.size(),
		.pBindings = bindings.data()
	});
	
	vk::PushConstantRange pushConstant {
		.stageFlags = vk::ShaderStageFlagBits::eFragment,
		.offset = 0,
		.size = sizeof(SlugGlyphData)
	};
	
	const vk::PipelineLayoutCreateInfo layoutInfo {
		.setLayoutCount = 1,
		.pSetLayouts = &*m_descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstant
	};
	
	m_pipelineLayout = vk::raii::PipelineLayout(p_context.GetDevice(), layoutInfo);
	
	m_shaderModules[0] = CreateShaderModule(p_context.GetDevice(), "slug_test.vert.spv");
	m_shaderModules[1] = CreateShaderModule(p_context.GetDevice(), "slug_test.frag.spv");
	
	m_frameResources.resize(2);
	for (FrameResources &frame : m_frameResources)
	{
		frame.cameraBuffer = CreateHostBuffer(p_allocator, sizeof(CameraUniform), vk::BufferUsageFlagBits::eUniformBuffer);

		const vk::DescriptorSetAllocateInfo allocateInfo {
			.descriptorPool = *m_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &*m_descriptorSetLayout
		};
		
		try
		{
			frame.descriptorSet = std::move(p_context.GetDevice().allocateDescriptorSets(allocateInfo).front());
		}
		catch (const std::exception &err)
		{
			std::println(std::cerr, "DEBUG ERROR: {0}", err.what());
			std::flush(std::cerr);
			throw std::runtime_error("");
		}
		
		const vk::DescriptorBufferInfo bufferInfo {
			.buffer = frame.cameraBuffer,
			.offset = 0,
			.range = sizeof(CameraUniform)
		};
		
		const vk::DescriptorImageInfo curveInfo {
			.sampler = nullptr,
			.imageView = m_curveTextureView->Get(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};
		
		const vk::DescriptorImageInfo bandInfo {
			.sampler = nullptr,
			.imageView = m_bandTextureView->Get(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};
		
		const vk::DescriptorBufferInfo glyphBufferInfo {
			.buffer = m_storageBuffer.Get(),
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};
		
		const std::array<vk::WriteDescriptorSet, 4> descriptorWrites {{
				{
					.dstSet = frame.descriptorSet,
					.dstBinding = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &bufferInfo
				},
				{
					.dstSet = frame.descriptorSet,
					.dstBinding = 1,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eSampledImage,
					.pImageInfo = &curveInfo
				},
				{
					.dstSet = frame.descriptorSet,
					.dstBinding = 2,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eSampledImage,
					.pImageInfo = &bandInfo
				},
				{
					.dstSet = frame.descriptorSet,
					.dstBinding = 3,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eStorageBuffer,
					.pBufferInfo = &glyphBufferInfo
				}
			}};

		p_context.GetDevice().updateDescriptorSets(descriptorWrites, {});
	}
	
	const auto createPipeline = [&](std::size_t p_shaderOffset)
	{
		const std::array<vk::PipelineShaderStageCreateInfo, 2> stages {{
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = *m_shaderModules[p_shaderOffset],
				.pName = "slugVertMain"
			},
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = *m_shaderModules[p_shaderOffset + 1],
				.pName = "slugFragMain"
			}
		}};
		
		const vk::PipelineVertexInputStateCreateInfo vertexInput {
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		const vk::PipelineInputAssemblyStateCreateInfo inputAssembly { 
			.topology = vk::PrimitiveTopology::eTriangleList 
		};

		const vk::PipelineViewportStateCreateInfo viewportState { 
			.viewportCount = 1, 
			.scissorCount = 1 
		};


		const vk::PipelineRasterizationStateCreateInfo rasterizer {
			.pNext = nullptr,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eCounterClockwise,
			.lineWidth = 1.5f
		};

		const vk::PipelineMultisampleStateCreateInfo multisampling { 
			.rasterizationSamples = vk::SampleCountFlagBits::e1
		};

		const vk::PipelineDepthStencilStateCreateInfo depthStencil {
			.depthTestEnable = vk::False,
			.depthWriteEnable = vk::True,
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

		const vk::PipelineColorBlendStateCreateInfo colorBlending {
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
				.pColorBlendState = &colorBlending,
				.pDynamicState = &dynamicState,
				.layout = *m_pipelineLayout
			},
			{
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &m_colorFormat,
				.depthAttachmentFormat = m_depthFormat
			}
		};

		return vk::raii::Pipeline(p_context.GetDevice(), nullptr, createInfo.get<vk::GraphicsPipelineCreateInfo>());
	};
	
	m_pipeline = createPipeline(0);
}

void Droplet::Graphics::VK::Slug::SlugPrototype::RenderToCamera(const Camera &p_camera,
	const vk::raii::CommandBuffer &p_commandBuffer, vk::Extent2D p_extent, std::uint32_t p_frameIndex)
{
	if (!m_device)
	{
		return;
	}

	if (p_frameIndex >= m_frameResources.size())
	{
		return;
	}

	if (p_extent.width == 0 || p_extent.height == 0)
	{
		return;
	}

	FrameResources &frame = m_frameResources[p_frameIndex];

	const CameraUniform cameraUniform { 
		p_camera.GetViewMatrix(), 
		p_camera.GetProjectionMatrix(),
		p_camera.GetAspect()
	};

	frame.cameraBuffer.getAllocation().copyFromMemory(&cameraUniform, 0, sizeof(cameraUniform));
	const vk::Viewport viewport {
		.x = 0.0f,
		.y = static_cast<float>(p_extent.height),
		.width = static_cast<float>(p_extent.width),
		.height = -static_cast<float>(p_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	const vk::Rect2D scissor { 
		vk::Offset2D(0, 0), 
		p_extent 
	};
	
	p_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
	p_commandBuffer.setViewport(0, viewport);
	p_commandBuffer.setScissor(0, scissor);
	p_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, *frame.descriptorSet, nullptr);
	p_commandBuffer.draw(6, m_glyphInstances.size(), 0,0);
}

void Droplet::Graphics::VK::Slug::SlugPrototype::Shutdown()
{
	if (m_device)
	{
		m_device->waitIdle();
	}

	m_frameResources.clear();
	m_pipeline = nullptr;

	m_pipelineLayout = nullptr;
	for (auto &shaderModule : m_shaderModules)
	{
		shaderModule = nullptr;
	}

	m_descriptorSetLayout = nullptr;

	m_device = nullptr;
	m_descriptorPool = nullptr;
	m_allocator = nullptr;
	m_colorFormat = {};
	m_depthFormat = {};
	m_storageBuffer = {};
	
	m_curveTextureView.reset();
	m_bandTextureView.reset();
	
	m_curveTexture.reset();
	m_bandTexture.reset();
}

bool Droplet::Graphics::VK::Slug::SlugPrototype::LoadFont(const char *p_inputPath)
{
	std::ifstream inFile(p_inputPath, std::ios::binary);
	if (!inFile.is_open())
	{
		std::cerr << "Failed to open font file." << std::endl;
		return false;
	}
	
	char header[SLUG_HEADER_LEN + 1];
	inFile.read(header, SLUG_HEADER_LEN);
	header[SLUG_HEADER_LEN] = '\0';
	if (strcmp(header, SLUG_HEADER_DATA) != 0)
	{
		std::cerr << "Invalid header found (" << header << " instead of " << SLUG_HEADER_DATA << ")" << std::endl;
		return false;
	}
	
	char version[SLUG_FILE_VERSION_LEN + 1];
	inFile.read(version, SLUG_FILE_VERSION_LEN);
	version[SLUG_FILE_VERSION_LEN] = '\0';
	if (strcmp(version, SLUG_FILE_VERSION) != 0)
	{
		std::cerr << "Version mismatch! Expected version number " << SLUG_FILE_VERSION << " but got " << version << std::endl;
		return false;
	}
	
	inFile.read(reinterpret_cast<char *>(&m_fontHeader), sizeof(SlugFontHeader));
	
	uint16_t codePointCount = 0;
	inFile.read(reinterpret_cast<char *>(&codePointCount), sizeof(codePointCount));
	if (codePointCount == 0)
	{
		std::cerr << "No code points found." << std::endl;
		return false;
	}
	
	m_codePoints.resize(codePointCount);
	inFile.read(reinterpret_cast<char *>(m_codePoints.data()), static_cast<std::streamsize>(m_codePoints.size() * sizeof(SlugCodePoint)));
	
	uint32_t curveTextureBytes;
	inFile.read(reinterpret_cast<char *>(&m_curveTextureWidth), sizeof(m_curveTextureWidth));
	inFile.read(reinterpret_cast<char *>(&m_curveTextureHeight), sizeof(m_curveTextureHeight));
	inFile.read(reinterpret_cast<char *>(&curveTextureBytes), sizeof(curveTextureBytes));
	if (m_curveTextureWidth == 0 || m_curveTextureHeight == 0 || curveTextureBytes == 0 || m_curveTextureWidth != TEXTURE_WIDTH)
	{
		std::cerr << "Invalid curve texture dimensions." << std::endl;
		return false;
	}
	
	// Number of texels in the curve texture
	const size_t curveTexTexels = static_cast<size_t>(m_curveTextureWidth * m_curveTextureHeight);
	m_curvesTextureData.resize(curveTexTexels);
	
	memset(m_curvesTextureData.data(), 0, m_curvesTextureData.size() * sizeof(m_curvesTextureData.front()));
	inFile.read(reinterpret_cast<char *>(m_curvesTextureData.data()), curveTextureBytes);
	
	uint32_t bandsTextureBytes;
	inFile.read(reinterpret_cast<char *>(&m_bandsTextureWidth), sizeof(m_bandsTextureWidth));
	inFile.read(reinterpret_cast<char *>(&m_bandsTextureHeight), sizeof(m_bandsTextureHeight));
	inFile.read(reinterpret_cast<char *>(&bandsTextureBytes), sizeof(bandsTextureBytes));
	if (m_bandsTextureWidth == 0 || m_bandsTextureHeight == 0 || bandsTextureBytes == 0 || m_bandsTextureWidth != TEXTURE_WIDTH)
	{
		std::cerr << "Invalid bands texture dimensions." << std::endl;
		return false;
	}
	
	
	// Number of texels in the bands texture
	const size_t bandsTexTexels = static_cast<size_t>(m_bandsTextureWidth * m_bandsTextureHeight);
	m_bandsTextureData.resize(bandsTexTexels);
	
	memset(m_bandsTextureData.data(), 0, m_bandsTextureData.size() * sizeof(m_bandsTextureData.front()));
	inFile.read(reinterpret_cast<char *>(m_bandsTextureData.data()), bandsTextureBytes);
	
	return true;
}

const Droplet::Graphics::VK::Slug::SlugCodePoint* Droplet::Graphics::VK::Slug::SlugPrototype::GetGlyph(const uint32_t p_codePoint) const
{
	const auto it = std::ranges::find(
		m_codePoints,
		p_codePoint,
		&SlugCodePoint::codePoint
	);
	
	return it != m_codePoints.end() ? &(*it) : nullptr;
}

std::vector<std::uint32_t> Droplet::Graphics::VK::Slug::SlugPrototype::DecodeUTF8(std::string_view p_text)
{
	std::vector<std::uint32_t> codePoints;

	for (std::size_t i = 0; i < p_text.size();)
	{
		const auto first = static_cast<std::uint8_t>(p_text[i]);

		std::uint32_t codePoint = 0;
		std::size_t length = 0;

		if (first < 0x80)
		{
			codePoint = first;
			length = 1;
		}
		else if ((first & 0xE0) == 0xC0)
		{
			codePoint = first & 0x1F;
			length = 2;
		}
		else if ((first & 0xF0) == 0xE0)
		{
			codePoint = first & 0x0F;
			length = 3;
		}
		else if ((first & 0xF8) == 0xF0)
		{
			codePoint = first & 0x07;
			length = 4;
		}
		else
		{
			throw std::runtime_error("Invalid UTF-8");
		}

		if (i + length > p_text.size())
			throw std::runtime_error("Incomplete UTF-8 sequence");

		for (std::size_t j = 1; j < length; ++j)
		{
			const auto byte = static_cast<std::uint8_t>(p_text[i + j]);

			if ((byte & 0xC0) != 0x80)
				throw std::runtime_error("Invalid UTF-8 continuation byte");

			codePoint = (codePoint << 6) | (byte & 0x3F);
		}

		if ((length == 2 && codePoint < 0x80) ||
			(length == 3 && codePoint < 0x800) ||
			(length == 4 && codePoint < 0x10000) ||
			(codePoint >= 0xD800 && codePoint <= 0xDFFF) ||
			codePoint > 0x10FFFF)
		{
			throw std::runtime_error("Invalid UTF-8 code point");
		}

		codePoints.push_back(codePoint);
		i += length;
	}

	return codePoints;
}
