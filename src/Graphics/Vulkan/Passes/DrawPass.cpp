/// @file
/// @brief Vulkan::DrawPass: the scene shader interface and the draw loop.

#include "Graphics/Vulkan/Passes/DrawPass.hpp"

#include "Graphics/Vulkan/Core/Context.hpp"
#include "Graphics/Vulkan/Frame/RenderDevice.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <variant>

#include "Graphics/Vulkan/Resources/MeshVertex.hpp"

namespace
{
	/// @brief Bytes one attribute of `format` takes
	std::uint32_t FormatBytes(Droplet::Common::VertexFormat format)
	{
		switch (format)
		{
		case Droplet::Common::VertexFormat::Float1: return 4;
		case Droplet::Common::VertexFormat::Float2: return 8;
		case Droplet::Common::VertexFormat::Float3: return 12;
		case Droplet::Common::VertexFormat::Float4: return 16;
		case Droplet::Common::VertexFormat::UNorm8x4: return 4;
		}
		return 0;
	}

	/// @brief The Vulkan format of a vertex attribute
	vk::Format ToVk(Droplet::Common::VertexFormat format)
	{
		switch (format)
		{
		case Droplet::Common::VertexFormat::Float1: return vk::Format::eR32Sfloat;
		case Droplet::Common::VertexFormat::Float2: return vk::Format::eR32G32Sfloat;
		case Droplet::Common::VertexFormat::Float3: return vk::Format::eR32G32B32Sfloat;
		case Droplet::Common::VertexFormat::Float4: return vk::Format::eR32G32B32A32Sfloat;
		case Droplet::Common::VertexFormat::UNorm8x4: return vk::Format::eR8G8B8A8Unorm;
		}
		return vk::Format::eUndefined;
	}
}

namespace Droplet::Graphics::Vulkan
{
	namespace
	{
		/// Every stage a built-in or asset shader can run in: the view UBO and push constants are visible to all of them
		constexpr vk::ShaderStageFlags ALL_SHADER_STAGES =
			vk::ShaderStageFlagBits::eVertex |
			vk::ShaderStageFlagBits::eTessellationControl |
			vk::ShaderStageFlagBits::eTessellationEvaluation |
			vk::ShaderStageFlagBits::eGeometry |
			vk::ShaderStageFlagBits::eFragment;
	}

	DrawPass::DrawPass(
		const Context& context,
		RenderDevice& device,
		const DrawPassCreateInfo& info)
		: m_context(&context),
		  m_gpu(&device.Resources()),
		  m_textures(&device.Textures())
	{
		// set 0 = the view's ViewProjectionBuffer, one dynamic offset per view
		constexpr vk::DescriptorSetLayoutBinding viewBinding{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
			.descriptorCount = 1,
			.stageFlags = ALL_SHADER_STAGES };
		m_viewSetLayout = vk::raii::DescriptorSetLayout(
			context.GetDevice(),
			vk::DescriptorSetLayoutCreateInfo{
				.bindingCount = 1,
				.pBindings = &viewBinding
			});
		m_reservedSetLayout = vk::raii::DescriptorSetLayout(context.GetDevice(), vk::DescriptorSetLayoutCreateInfo{});

		const std::array setLayouts{ *m_viewSetLayout, *m_reservedSetLayout, m_textures->Layout() };
		constexpr vk::PushConstantRange pushRange{
			.stageFlags = ALL_SHADER_STAGES,
			.offset = 0,
			.size = sizeof(Common::PushConstantData)
		};
		m_pipelineLayout = vk::raii::PipelineLayout(
			context.GetDevice(),
			vk::PipelineLayoutCreateInfo{
				.setLayoutCount = static_cast<std::uint32_t>(setLayouts.size()),
				.pSetLayouts = setLayouts.data(),
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &pushRange
			});

		// the view sets point at their slot's buffer once it exists (BeginFrame)
		const vk::DeviceSize alignment = context.GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_viewUniformStride = (sizeof(Common::ViewProjectionBuffer) + alignment - 1) / alignment * alignment;

		const vk::DescriptorPoolSize viewPoolSize{
			.type = vk::DescriptorType::eUniformBufferDynamic,
			.descriptorCount = info.framesInFlight
		};
		m_viewPool = vk::raii::DescriptorPool(
			context.GetDevice(),
			vk::DescriptorPoolCreateInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = info.framesInFlight,
				.poolSizeCount = 1,
				.pPoolSizes = &viewPoolSize
			});
		const std::vector<vk::DescriptorSetLayout> viewLayouts(info.framesInFlight, *m_viewSetLayout);
		m_viewSets = vk::raii::DescriptorSets(context.GetDevice(), vk::DescriptorSetAllocateInfo{
			.descriptorPool = *m_viewPool,
			.descriptorSetCount = info.framesInFlight,
			.pSetLayouts = viewLayouts.data()
		});
		m_viewBuffers.resize(info.framesInFlight);
		m_dynamicVertexBuffers.resize(info.framesInFlight);
		m_dynamicIndexBuffers.resize(info.framesInFlight);
	}

	Common::ShaderDesc DrawPass::VertexColorShaderAsset()
	{
		return Common::ShaderDesc{
			.moduleName = "vertex_color",
			.sourcePath = ROOT_PATH "/Assets/Shaders/vertex_color.slang",
			.vertexEntry = "vMain",
			.fragmentEntry = "fMain"
		};
	}

	void DrawPass::BeginFrame(std::uint32_t frameSlot, const Render::PreparedFrame& frame)
	{
		ZoneScoped
		m_frameSlot = frameSlot;
		m_time = frame.time;

		// the slot's buffer is not in use any more: grow it in place, and point the slot's set at the new one
		Buffer& buffer = m_viewBuffers[frameSlot];
		const vk::DeviceSize needed = std::max<vk::DeviceSize>(frame.views.size(), 1) * m_viewUniformStride;
		if (!buffer || buffer.GetSize() < needed)
		{
			buffer = Buffer::Create(m_context->GetAllocator(),
									BufferCreateOptions::Uniform(std::max(needed * 2, 8 * m_viewUniformStride)));

			const vk::DescriptorBufferInfo bufferInfo{
				.buffer = buffer.Get(),
				.offset = 0,
				.range = sizeof(Common::ViewProjectionBuffer)
			};
			m_context->GetDevice().updateDescriptorSets(vk::WriteDescriptorSet{
				.dstSet = *m_viewSets[frameSlot],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
				.pBufferInfo = &bufferInfo
			}, {});
		}

		// the frame's dynamic geometry; the slot's buffers are not in use any more, so a small one is replaced
		const auto writeStream = [this](Buffer& stream, const void* data, vk::DeviceSize byteSize, bool isIndex)
		{
			if (byteSize == 0)
				return;
			if (!stream || stream.GetSize() < byteSize)
			{
				const vk::DeviceSize capacity = std::max<vk::DeviceSize>(byteSize * 2, 64 * 1024);
				stream = Buffer::Create(m_context->GetAllocator(), isIndex
					? BufferCreateOptions::Index(capacity, MemoryAccess::Dynamic)
					: BufferCreateOptions::Vertex(capacity, MemoryAccess::Dynamic));
			}
			stream.Write(data, byteSize);
		};
		writeStream(m_dynamicVertexBuffers[frameSlot], frame.dynamicVertexData.data(), frame.dynamicVertexData.size(), false);
		writeStream(m_dynamicIndexBuffers[frameSlot], frame.dynamicIndices.data(),
					frame.dynamicIndices.size() * sizeof(std::uint32_t), true);

		for (std::size_t viewIndex = 0; viewIndex < frame.views.size(); ++viewIndex)
		{
			const Render::DrawList& view = frame.views[viewIndex];
			const Common::ViewProjectionBuffer uniforms{ .view = view.view, .proj = view.proj };
			buffer.Write(&uniforms, sizeof(uniforms), viewIndex * m_viewUniformStride);
		}
	}

	GraphicsPipelineDesc DrawPass::MakePipelineDesc(
		GUID vertexShader, GUID fragmentShader, GUID tessCtrlShader, GUID tessEvalShader, GUID geometryShader,
		vk::Format colorFormat, vk::Format depthFormat, bool transparent) const
	{
		ZoneScoped
		constexpr auto binding = Vertex::P3N3UV2::GetBindingDescription();
		constexpr auto attributes = Vertex::P3N3UV2::GetAttributeDescription();

		// Depth range 0..1, cleared to 1.0, so "less" keeps the nearest fragment
		const bool hasDepth = depthFormat != vk::Format::eUndefined;
		// tessellation draws patches, not triangles
		const bool tessellation = tessCtrlShader != C_INVALID_GUID || tessEvalShader != C_INVALID_GUID;

		return GraphicsPipelineDesc{
			.vertexShader = vertexShader,
			.fragmentShader = fragmentShader,
			.tessCtrlShader = tessCtrlShader,
			.tessEvalShader = tessEvalShader,
			.geometryShader = geometryShader,
			.hasFragmentShader = true,
			.hasTessCtrlShader = tessCtrlShader != C_INVALID_GUID,
			.hasTessEvalShader = tessEvalShader != C_INVALID_GUID,
			.hasGeometryShader = geometryShader != C_INVALID_GUID,
			.layout = *m_pipelineLayout,
			.vertexLayout = VertexLayout{
				.bindings = {binding},
				.attributes = {attributes.begin(), attributes.end()}
			},
			.topology = tessellation ? vk::PrimitiveTopology::ePatchList : vk::PrimitiveTopology::eTriangleList,
			.raster = RasterState{
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eBack,
				.frontFace = vk::FrontFace::eCounterClockwise // the viewport's Y is flipped
			},
			.depth = DepthState{
				.testEnable = hasDepth,
				.writeEnable = hasDepth && !transparent, // transparent objects test against the opaque ones only
				.compareOp = vk::CompareOp::eLess
			},
			.colorFormats = {colorFormat},
			.colorBlend = {transparent ? BlendMode::AlphaBlend : BlendMode::Opaque},
			// No stencil: the pass attaches depth only, and the pipeline's formats must match the rendering info.
			.depthFormat = depthFormat,
			.samples = vk::SampleCountFlagBits::e1
		};
	}

	DrawPass::MeshInterface DrawPass::InterfaceOf(const Common::DynamicMeshData& mesh)
	{
		ZoneScoped
		using Droplet::Common::VertexFormat;
		const auto formatAt = [&mesh](std::uint32_t location) -> std::optional<VertexFormat>
		{
			const auto attribute = std::ranges::find(mesh.attributes, location, &Common::VertexAttribute::location);
			return attribute != mesh.attributes.end() ? std::optional{ attribute->format } : std::nullopt;
		};
		const bool fits = std::ranges::all_of(mesh.attributes, [&mesh](const Common::VertexAttribute& attribute)
		{
			return attribute.offset + FormatBytes(attribute.format) <= mesh.vertexStride;
		});
		if (!fits || formatAt(0) != VertexFormat::Float3)
			return MeshInterface::None;
		if (formatAt(1) == VertexFormat::Float3 && formatAt(2) == VertexFormat::Float2)
			return MeshInterface::Scene;
		// particles: centre, colour, size, rotation; must precede VertexColor, which also matches colour at 1
		if (mesh.topology == Common::MeshTopology::Points &&
			formatAt(1) == VertexFormat::Float4 && formatAt(2) == VertexFormat::Float1 && formatAt(3) == VertexFormat::Float1)
			return MeshInterface::Particle;
		if (formatAt(1) == VertexFormat::Float4 || formatAt(1) == VertexFormat::UNorm8x4)
			return MeshInterface::VertexColor;
		return MeshInterface::None;
	}

	GraphicsPipelineDesc DrawPass::MakeMeshPipelineDesc(
		GUID vertexShader, GUID fragmentShader, GUID tessCtrlShader, GUID tessEvalShader, GUID geometryShader,
		const Common::DynamicMeshData& mesh,
		vk::Format colorFormat, vk::Format depthFormat, bool transparent) const
	{
		ZoneScoped
		const MeshInterface meshInterface = InterfaceOf(mesh);
		const bool vertexColor = meshInterface == MeshInterface::VertexColor;
		const bool particle = meshInterface == MeshInterface::Particle;
		GraphicsPipelineDesc desc = MakePipelineDesc(
			vertexShader, fragmentShader, tessCtrlShader, tessEvalShader, geometryShader, colorFormat, depthFormat,
			transparent || vertexColor || particle);

		desc.vertexLayout.bindings = { vk::VertexInputBindingDescription{
			.binding = 0, .stride = mesh.vertexStride, .inputRate = vk::VertexInputRate::eVertex } };
		desc.vertexLayout.attributes.clear();
		for (const Common::VertexAttribute& attribute : mesh.attributes)
		{
			desc.vertexLayout.attributes.push_back({
				.location = attribute.location, .binding = 0, .format = ToVk(attribute.format), .offset = attribute.offset });
		}
		if (mesh.topology == Common::MeshTopology::Points)
			desc.topology = vk::PrimitiveTopology::ePointList;
		if (particle)
		{
			// the geometry shader expands each point into a quad facing the view: both faces are visible
			desc.raster.cullMode = vk::CullModeFlagBits::eNone;
		}
		if (mesh.topology == Common::MeshTopology::Lines)
		{
			// as wide as production's debug lines were, where the device allows
			constexpr float DEBUG_LINE_WIDTH = 1.5f;
			desc.topology = vk::PrimitiveTopology::eLineList;
			desc.raster.lineWidth = std::min(DEBUG_LINE_WIDTH, m_context->GetMaxLineWidth());
		}
		if (vertexColor)
		{
			// as production's debug drawing: both faces, depth written where tested, equal depths pass
			desc.raster.cullMode = vk::CullModeFlagBits::eNone;
			desc.depth.writeEnable = desc.depth.testEnable;
			desc.depth.compareOp = vk::CompareOp::eLessOrEqual;
		}
		if (!mesh.depthTest)
		{
			desc.depth.testEnable = false;
			desc.depth.writeEnable = false;
		}
		return desc;
	}

	std::uint32_t DrawPass::TextureIndexOf(const Render::ImageBinding& binding) const
	{
		if (const Render::TextureHandle* textureHandle = std::get_if<Render::TextureHandle>(&binding))
		{
			if (const GpuTexture* texture = m_gpu->Find(*textureHandle))
				return texture->slot.Index();
		}
		else if (const Render::TargetHandle* targetHandle = std::get_if<Render::TargetHandle>(&binding))
		{
			if (const GpuRenderTarget* target = m_gpu->Find(*targetHandle))
				return target->slot.Index();
		}
		// stale handle (see Record): the placeholder texture, so the missing image is obvious
		return m_gpu->Find(m_gpu->PlaceholderTexture())->slot.Index();
	}

	void DrawPass::Record(
		vk::CommandBuffer cmd,
		vk::Extent2D extent,
		const Render::DrawList& list,
		std::uint32_t viewIndex,
		const PipelineLibrary& pipelines,
		DrawFilter filter)
	{
		ZoneScoped
		// Same Y flip as the existing scene pass: negative height with the origin moved to the bottom.
		const vk::Viewport viewport{
			.x = 0.0f,
			.y = static_cast<float>(extent.height),
			.width = static_cast<float>(extent.width),
			.height = -static_cast<float>(extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		const vk::Rect2D scissor{
			.offset = {
				.x = 0,
				.y = 0
			},
			.extent = extent
		};
		cmd.setViewport(0, viewport);
		cmd.setScissor(0, scissor);

		// the view's uniforms and the texture table stay bound for every draw (one pipeline layout)
		const std::uint32_t viewOffset = static_cast<std::uint32_t>(viewIndex * m_viewUniformStride);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, *m_viewSets[m_frameSlot], viewOffset);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 2, m_textures->Set(m_frameSlot), {});

		// what every draw of the view pushes besides its world matrix and texture
		Common::PushConstantData pushConstants{
			.cameraDir = { list.cameraForward.x, list.cameraForward.y, list.cameraForward.z, 0.0f },
			.cameraPos = { list.cameraPosition.x, list.cameraPosition.y, list.cameraPosition.z, 1.0f },
			.time = m_time
		};
		const auto push = [&](const glm::mat4& world, const Render::ImageBinding& material)
		{
			pushConstants.world = world;
			pushConstants.textureID = TextureIndexOf(material);
			cmd.pushConstants<Common::PushConstantData>(
				*m_pipelineLayout, ALL_SHADER_STAGES, 0, pushConstants);
		};

		Render::PipelineHandle boundPipeline{};
		const auto bindPipeline = [&](Render::PipelineHandle handle)
		{
			if (handle == boundPipeline)
				return true;
			const GraphicsPipeline* pipeline = pipelines.Find(handle);
			if (!pipeline)
				return false;
			pipeline->Bind(cmd);
			boundPipeline = handle;
			return true;
		};

		// objects and kept dynamic meshes: their own vertex and index buffers
		const auto drawItems = [&](const std::vector<Render::DrawItem>& items)
		{
			for (const Render::DrawItem& item : items)
			{
				if (filter == DrawFilter::SkipRenderTargetTextures
					&& std::holds_alternative<Render::TargetHandle>(item.material))
					continue;

				// Prepare resolved every handle this frame, so these lookups only fail for a list kept past its frame
				// (a stale handle): skip rather than crash, or draw with a pipeline whose attachment formats need not match
				const GpuMesh* mesh = m_gpu->Find(item.mesh);
				if (!mesh || !bindPipeline(item.pipeline))
					continue;
				push(item.model, item.material);

				cmd.bindVertexBuffers(0, mesh->vertexBuffer.Get(), vk::DeviceSize{ 0 });
				if (mesh->indexCount == 0)
				{
					cmd.draw(mesh->vertexCount, 1, 0, 0); // drawn in vertex order
					continue;
				}
				cmd.bindIndexBuffer(mesh->indexBuffer.Get(), 0, mesh->indexType);
				cmd.drawIndexed(mesh->indexCount, 1, 0, 0, 0);
			}
		};
		{
			const GpuZone zone{m_context->GetGpuProfiler(), QueueKind::Graphics, cmd, "Draw kept meshes"};
			drawItems(list.items);
		}

		// dynamic geometry, from this frame's stream buffers (BeginFrame)
		if (!list.dynamicItems.empty())
		{
			const GpuZone zone{m_context->GetGpuProfiler(), QueueKind::Graphics, cmd, "Draw streamed meshes"};
			const Buffer& vertexStream = m_dynamicVertexBuffers[m_frameSlot];
			const Buffer& indexStream = m_dynamicIndexBuffers[m_frameSlot];
			if (indexStream)
				cmd.bindIndexBuffer(indexStream.Get(), 0, vk::IndexType::eUint32);
			std::uint32_t boundVertexOffset = ~0u;
			for (const Render::DynamicDrawItem& item : list.dynamicItems)
			{
				if (!vertexStream || (item.indexed && !indexStream))
					break; // nothing was written this frame: a list kept past its frame
				if (!bindPipeline(item.pipeline))
					continue;
				push(item.model, item.material);

				if (item.vertexByteOffset != boundVertexOffset)
				{
					cmd.bindVertexBuffers(0, vertexStream.Get(), vk::DeviceSize{ item.vertexByteOffset });
					boundVertexOffset = item.vertexByteOffset;
				}
				if (item.indexed)
					cmd.drawIndexed(item.count, 1, item.first, 0, 0);
				else
					cmd.draw(item.count, 1, item.first, 0);
			}
		}

		// kept dynamic meshes without depth test, over everything
		{
			const GpuZone zone{m_context->GetGpuProfiler(), QueueKind::Graphics, cmd, "Draw late meshes (no depth)"};
			drawItems(list.lateItems);
		}
	}
}
