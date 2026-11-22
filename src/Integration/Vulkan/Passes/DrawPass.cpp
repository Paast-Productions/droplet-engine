/// @file
/// @brief Vulkan::DrawPass: the scene material interface, material descriptor sets and the draw loop.

#include "Integration/Vulkan/Passes/DrawPass.hpp"

#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Frame/RenderDevice.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <variant>

#include "Integration/Vulkan/Resources/MeshVertex.hpp"

namespace Vulkan
{
    DrawPass::DrawPass(
        const Context& context,
        RenderDevice& device,
        const DrawPassCreateInfo& info)
        : m_context(&context), 
    m_gpu(&device.Resources())
	{
        // the sets ofjag evicted or replaced textures and targets go once the GPU is past this frame
        device.OnResourcesReleased([this, &device](const QueueTimelineValues& frameRetireValues)
            {
                RetireDeadMaterials(device.Deletions(), frameRetireValues);
            });

		// set 0 = the object's texture; push constants = view-projection and model matrices
		constexpr vk::DescriptorSetLayoutBinding textureBinding{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment };

		m_setLayout = vk::raii::DescriptorSetLayout(
			context.GetDevice(),
			vk::DescriptorSetLayoutCreateInfo{
				.bindingCount = 1,
				.pBindings = &textureBinding
			});

		// set 1 = the view's uniforms, one dynamic offset per view
		constexpr vk::DescriptorSetLayoutBinding viewBinding{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBufferDynamic,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment };

		m_viewSetLayout = vk::raii::DescriptorSetLayout(
			context.GetDevice(),
			vk::DescriptorSetLayoutCreateInfo{
				.bindingCount = 1,
				.pBindings = &viewBinding
			});

		const std::array setLayouts{ *m_setLayout, *m_viewSetLayout };
		constexpr vk::PushConstantRange pushRange{
			.stageFlags = vk::ShaderStageFlagBits::eVertex,
			.offset = 0,
			.size = sizeof(ScenePushConstants)
		};

		m_pipelineLayout = vk::raii::PipelineLayout(
			context.GetDevice(),
			vk::PipelineLayoutCreateInfo{
				.setLayoutCount = static_cast<std::uint32_t>(setLayouts.size()),
				.pSetLayouts = setLayouts.data(),
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &pushRange
			});

		// eFreeDescriptorSet: the raii descriptor sets free themselves back to the pool.
		const vk::DescriptorPoolSize poolSize{
			.type = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = info.maxMaterials
		};
		m_pool = vk::raii::DescriptorPool(
			context.GetDevice(),
			vk::DescriptorPoolCreateInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = info.maxMaterials,
				.poolSizeCount = 1,
				.pPoolSizes = &poolSize
			});

		// the view sets point at their slot's buffer once it exists (BeginFrame)
		const vk::DeviceSize alignment = context.GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_viewUniformStride = (sizeof(SceneViewUniforms) + alignment - 1) / alignment * alignment;

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
			.sourcePath = "Shaders/vertex_color.slang",
			.vertexEntry = "vMain",
			.fragmentEntry = "fMain"
		};
	}

	void DrawPass::BeginFrame(std::uint32_t frameSlot, const Render::PreparedFrame& frame)
	{
		m_frameSlot = frameSlot;

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
				.range = sizeof(SceneViewUniforms)
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
			const SceneViewUniforms uniforms{
				.cameraPosition = glm::vec4(view.cameraPosition, 1.0f),
				.cameraForward = glm::vec4(view.cameraForward, 0.0f),
				.time = frame.time
			};
			buffer.Write(&uniforms, sizeof(uniforms), viewIndex * m_viewUniformStride);
		}
	}

	GraphicsPipelineDesc DrawPass::MakePipelineDesc(
		Droplet::GUID shader, vk::Format colorFormat, vk::Format depthFormat, bool transparent) const
	{
		constexpr auto binding = Vertex::P3N3UV2::GetBindingDescription();
		constexpr auto attributes = Vertex::P3N3UV2::GetAttributeDescription();

		// Depth range 0..1 (glm::perspectiveRH_ZO), cleared to 1.0, so "less" keeps the nearest fragment
		const bool hasDepth = depthFormat != vk::Format::eUndefined;

		return GraphicsPipelineDesc{
			.vertexShader = shader,
			.fragmentShader = shader,
			.hasFragmentShader = true,
			.layout = *m_pipelineLayout,
			.vertexLayout = VertexLayout{
				.bindings = {binding},
				.attributes = {attributes.begin(), attributes.end()}
			},
			.topology = vk::PrimitiveTopology::eTriangleList,
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

	GraphicsPipelineDesc DrawPass::MakeDynamicPipelineDesc(
		Droplet::GUID shader, Render::DynamicVertexLayout layout, Render::DynamicTopology topology,
		vk::Format colorFormat, vk::Format depthFormat, bool transparent, bool depthTest) const
	{
		const bool vertexColor = layout == Render::DynamicVertexLayout::P3C4;
		GraphicsPipelineDesc desc = MakePipelineDesc(shader, colorFormat, depthFormat, transparent || vertexColor);
		if (vertexColor)
		{
			// as production's debug drawing: both faces, depth written where tested, equal depths pass
			constexpr auto binding = Vertex::P3C4::GetBindingDescription();
			constexpr auto attributes = Vertex::P3C4::GetAttributeDescription();
			desc.vertexLayout = VertexLayout{ .bindings = {binding}, .attributes = {attributes.begin(), attributes.end()} };
			desc.raster.cullMode = vk::CullModeFlagBits::eNone;
			desc.depth.writeEnable = desc.depth.testEnable;
			desc.depth.compareOp = vk::CompareOp::eLessOrEqual;
		}
		if (topology == Render::DynamicTopology::Lines)
			desc.topology = vk::PrimitiveTopology::eLineList;
		if (!depthTest)
		{
			desc.depth.testEnable = false;
			desc.depth.writeEnable = false;
		}
		return desc;
	}

	vk::raii::DescriptorSet DrawPass::CreateSet(vk::ImageView view, vk::Sampler sampler) const
	{
		const vk::DescriptorSetLayout layout = *m_setLayout;
		const vk::DescriptorSetAllocateInfo allocInfo{
			.descriptorPool = *m_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &layout
		};
		vk::raii::DescriptorSets sets(m_context->GetDevice(), allocInfo);
		vk::raii::DescriptorSet set = std::move(sets.front());

		// Written once, before the set is ever bound, so no in-flight command buffer can be using it.
		const vk::DescriptorImageInfo imageInfo{
			.sampler = sampler,
			.imageView = view,
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		const vk::WriteDescriptorSet write{
			.dstSet = *set,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &imageInfo
		};

		m_context->GetDevice().updateDescriptorSets(write, {});
		return set;
	}

	vk::DescriptorSet DrawPass::MaterialSet(const Render::ImageBinding& binding)
	{
		auto* cache = &m_textureSets;
		std::uint64_t key = 0;
		vk::ImageView view{};
		vk::Sampler sampler = m_gpu->DefaultSampler(); // render targets: one level, default filtering

		if (const Render::TextureHandle* textureHandle = std::get_if<Render::TextureHandle>(&binding))
		{
			if (const GpuTexture* texture = m_gpu->Find(*textureHandle))
			{
				view = texture->view.Get();
				sampler = texture->sampler;
				key = textureHandle->Key();
			}
		}
		else if (const Render::TargetHandle* targetHandle = std::get_if<Render::TargetHandle>(&binding))
		{
			if (const GpuRenderTarget* target = m_gpu->Find(*targetHandle))
			{
				view = target->target.GetImageView();
				key = targetHandle->Key();
				cache = &m_targetSets;
			}
		}
        if (!view) // stale handle (see Record): draw the placeholder texture, so the missing image is obvious
        {
            const Render::TextureHandle placeholder = m_gpu->PlaceholderTexture();
            const GpuTexture* texture = m_gpu->Find(placeholder);
            view = texture->view.Get();
            sampler = texture->sampler;
            key = placeholder.Key();
			cache = &m_textureSets;
		}

		if (const auto cachedSet = cache->find(key); cachedSet != cache->end())
			return *cachedSet->second;
		return *cache->emplace(key, CreateSet(view, sampler)).first->second;
	}

	void DrawPass::Record(
		vk::CommandBuffer cmd,
		vk::Extent2D extent,
		const Render::DrawList& list,
		std::uint32_t viewIndex,
		const PipelineLibrary& pipelines,
		DrawFilter filter)
	{
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

		// the view's uniforms stay bound across the material binds of set 0 (same pipeline layout)
		const std::uint32_t viewOffset = static_cast<std::uint32_t>(viewIndex * m_viewUniformStride);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 1, *m_viewSets[m_frameSlot], viewOffset);

		// every scene pipeline shares m_pipelineLayout, so bound descriptor sets stay valid across pipeline binds
		Render::PipelineHandle boundPipeline{};
		vk::DescriptorSet bound{};
		for (const Render::DrawItem& item : list.items)
		{
			if (filter == DrawFilter::SkipRenderTargetTextures
				&& std::holds_alternative<Render::TargetHandle>(item.material))
				continue;

			// Prepare resolved every handle this frame, so these lookups only fail for a list kept past its frame
			// (a stale handle): skip rather than crash, or draw with a pipeline whose attachment formats need not match
			const GpuMesh* mesh = m_gpu->Find(item.mesh);
			if (!mesh)
				continue;
			if (item.pipeline != boundPipeline)
			{
				const GraphicsPipeline* pipeline = pipelines.Find(item.pipeline);
				if (!pipeline)
					continue;
				pipeline->Bind(cmd);
				boundPipeline = item.pipeline;
			}

			const vk::DescriptorSet set = MaterialSet(item.material);
			if (set != bound)
			{
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, set, {});
				bound = set;
			}

			const ScenePushConstants pushConstants{list.viewProj, item.model};
			cmd.pushConstants<ScenePushConstants>(
				*m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstants);

			cmd.bindVertexBuffers(0, mesh->vertexBuffer.Get(), vk::DeviceSize{ 0 });
			cmd.bindIndexBuffer(mesh->indexBuffer.Get(), 0, mesh->indexType);
			cmd.drawIndexed(mesh->indexCount, 1, 0, 0, 0);
		}

		// dynamic geometry, from this frame's stream buffers (BeginFrame)
		if (list.dynamicItems.empty())
			return;
		const Buffer& vertexStream = m_dynamicVertexBuffers[m_frameSlot];
		const Buffer& indexStream = m_dynamicIndexBuffers[m_frameSlot];
		if (indexStream)
			cmd.bindIndexBuffer(indexStream.Get(), 0, vk::IndexType::eUint32);
		std::uint32_t boundVertexOffset = ~0u;
		for (const Render::DynamicDrawItem& item : list.dynamicItems)
		{
			if (!vertexStream || (item.indexed && !indexStream))
				break; // nothing was written this frame: a list kept past its frame
			if (item.pipeline != boundPipeline)
			{
				const GraphicsPipeline* pipeline = pipelines.Find(item.pipeline);
				if (!pipeline)
					continue;
				pipeline->Bind(cmd);
				boundPipeline = item.pipeline;
			}

			const vk::DescriptorSet set = MaterialSet(item.material);
			if (set != bound)
			{
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, set, {});
				bound = set;
			}

			const ScenePushConstants pushConstants{list.viewProj, item.model};
			cmd.pushConstants<ScenePushConstants>(
				*m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstants);

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

	void DrawPass::RetireDeadMaterials(
		DeletionQueue& deletionQueue, const QueueTimelineValues& frameRetireValues)
	{
		// a set may be bound by this frame's command buffers, so it lives until the GPU is past the frame
		std::erase_if(m_textureSets, [&](auto& entry)
			{
				if (m_gpu->Find(Render::TextureHandle::FromKey(entry.first)))
					return false;
				deletionQueue.Retire(std::move(entry.second), frameRetireValues);
				return true;
			});
		std::erase_if(m_targetSets, [&](auto& entry)
			{
				if (m_gpu->Find(Render::TargetHandle::FromKey(entry.first)))
					return false;
				deletionQueue.Retire(std::move(entry.second), frameRetireValues);
				return true;
			});
	}

	void DrawPass::ClearMaterialCache()
	{
		m_textureSets.clear();
		m_targetSets.clear();
	}
}
