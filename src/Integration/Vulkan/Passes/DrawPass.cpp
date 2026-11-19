/// @file
/// @brief Vulkan::DrawPass: the scene material interface, material descriptor sets and the draw loop.

#include "Integration/Vulkan/Passes/DrawPass.hpp"

#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Frame/RenderDevice.hpp"

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

		const vk::DescriptorSetLayout setLayout = *m_setLayout;
		constexpr vk::PushConstantRange pushRange{
			.stageFlags = vk::ShaderStageFlagBits::eVertex,
			.offset = 0,
			.size = sizeof(ScenePushConstants)
		};

		m_pipelineLayout = vk::raii::PipelineLayout(
			context.GetDevice(),
			vk::PipelineLayoutCreateInfo{
				.setLayoutCount = 1,
				.pSetLayouts = &setLayout,
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
	}

	GraphicsPipelineDesc DrawPass::MakePipelineDesc(
		const Guid& shader, vk::Format colorFormat, vk::Format depthFormat) const
	{
		constexpr auto binding = Vertex::P3N3UV2::GetBindingDescription();
		constexpr auto attributes = Vertex::P3N3UV2::GetAttributeDescription();

		// Depth range 0..1 (glm::perspectiveRH_ZO), cleared to 1.0, so "less" keeps the nearest fragment.
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
				.writeEnable = hasDepth,
				.compareOp = vk::CompareOp::eLess
			},
			.colorFormats = {colorFormat},
			.colorBlend = {BlendMode::Opaque},
			// No stencil: the pass attaches depth only, and the pipeline's formats must match the rendering info.
			.depthFormat = depthFormat,
			.samples = vk::SampleCountFlagBits::e1
		};
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
