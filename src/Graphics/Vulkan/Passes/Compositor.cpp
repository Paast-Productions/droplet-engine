/**
 * @file Compositor.cpp
 * @brief Vulkan::Compositor implementation: the fullscreen pipelines and their per-frame-slot descriptor pools.
 *        The passes themselves are render-graph passes (FrameGraphBuilder.cpp).
 */
#include "Graphics/Vulkan/Passes/Compositor.hpp"

#include <algorithm>
#include <array>
#include <cassert>

#include "Graphics/Vulkan/Core/Context.hpp"
#include "Graphics/Vulkan/Core/VulkanUtils.hpp"

namespace
{
    /// @brief Wraps SPIR-V words in a shader module.
    vk::raii::ShaderModule CreateShaderModule(const vk::raii::Device& device, const std::vector<std::uint32_t>& spirv)
    {
        const vk::ShaderModuleCreateInfo createInfo{
            .codeSize = spirv.size() * sizeof(std::uint32_t),
            .pCode = spirv.data()
        };

        return {device, createInfo};
    }
}

using namespace Droplet::Graphics::Vulkan;

Compositor::Compositor(const Context& context, std::uint32_t framesInFlight)
    : m_context{&context},
      m_framesInFlight{framesInFlight}
{
    CreateDescriptorSetLayout();
    CreateSampler();
    CreateDescriptorPools();
    CreatePipelineLayout();
}

void Compositor::CreateShaderModules(
    const std::vector<std::uint32_t>& vertexSpirv,
    const std::vector<std::uint32_t>& fragmentSpirv)
{
    m_vertexShaderModule = CreateShaderModule(Device(), vertexSpirv);
    m_fragmentShaderModule = CreateShaderModule(Device(), fragmentSpirv);
    m_pipelines.clear(); // rebuilt with the new shaders on next use
}

vk::Pipeline Compositor::PipelineFor(vk::Format colorFormat)
{
    if (const auto pipelineIt = m_pipelines.find(colorFormat); pipelineIt != m_pipelines.end())
        return *pipelineIt->second;

    const std::array shaderStages{
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = m_vertexShaderModule,
            .pName = "vMain"
        },
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = m_fragmentShaderModule,
            .pName = "fMain"
        }
    };

    constexpr std::array dynamicStates{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
        .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    // no vertex attribute or buffers
    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    const vk::PipelineViewportStateCreateInfo viewportStateInfo{
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    const vk::PipelineRasterizationStateCreateInfo rasterStateInfo{
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .lineWidth = 1.0f
    };

    const vk::PipelineMultisampleStateCreateInfo multisampleInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1
    };

    const vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendInfo{
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    const vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateChain{
        {
            .stageCount = static_cast<std::uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pViewportState = &viewportStateInfo,
            .pRasterizationState = &rasterStateInfo,
            .pMultisampleState = &multisampleInfo,
            .pColorBlendState = &colorBlendInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = *m_pipelineLayout,
            .renderPass = nullptr
        },
        {
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = vk::Format::eUndefined,
            .stencilAttachmentFormat = vk::Format::eUndefined
        }
    };

    // dynamic rendering: the colour format is part of the pipeline, hence one per format
    const auto [pipelineIt, inserted] = m_pipelines.emplace(
        colorFormat,
        vk::raii::Pipeline(Device(), nullptr, pipelineCreateChain.get<vk::GraphicsPipelineCreateInfo>()));
    return *pipelineIt->second;
}

void Compositor::BeginFrame(std::uint32_t frameSlot)
{
    assert(frameSlot < m_framesInFlight);
    m_frameSlot = frameSlot;

    // every set allocated from this pool belonged to the frame that last used the slot, which the GPU has finished
    m_framePools[frameSlot].reset();
}

void Compositor::RecordFullscreen(
    vk::CommandBuffer commandBuffer,
    vk::ImageView source,
    vk::ImageView destination,
    vk::Format destinationFormat,
    vk::Extent2D extent,
    const PostProcess::PushConstants& pushConstants)
{
    const vk::RenderingAttachmentInfo colorAttachment{
        .imageView = destination,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = m_defaultClearColor
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {
            .offset = {.x = 0, .y = 0},
            .extent = extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    // -- begin render pass --
    commandBuffer.beginRendering(renderingInfo);

    // no source: the clear (loadOp) is the whole pass
    if (source)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, PipelineFor(destinationFormat));

        // viewport and scissors are dynamic so they need to be set every time
        const vk::Viewport viewport{
            .x = 0.0f,
            .y = 0.0f, // static_cast<float>(destinationExtent.height), // move the offset to account for the flipped Y
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height), // negative to flip the Y coordinate for Vulkan
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        const vk::Rect2D scissor{
            .offset = {.x = 0, .y = 0},
            .extent = extent
        };

        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *m_pipelineLayout,
            0,
            AllocateSourceSet(source),
            nullptr);

        commandBuffer.pushConstants(
            *m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(PostProcess::PushConstants),
            &pushConstants);

        commandBuffer.draw(3, 1, 0, 0);
    }

    commandBuffer.endRendering();
}

void Compositor::CreateDescriptorSetLayout()
{
    constexpr vk::DescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment
    };

    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = 1,
        .pBindings = &binding
    };

    m_descriptorSetLayout = vk::raii::DescriptorSetLayout(Device(), createInfo);
}

void Compositor::CreateSampler()
{
    constexpr vk::SamplerCreateInfo createInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,
        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
        .addressModeV = vk::SamplerAddressMode::eClampToEdge,
        .addressModeW = vk::SamplerAddressMode::eClampToEdge,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::False,
        .maxAnisotropy = 1.0f,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eFloatOpaqueBlack,
        .unnormalizedCoordinates = vk::False
    };

    m_sampler = vk::raii::Sampler{Device(), createInfo};
}

void Compositor::CreateDescriptorPools()
{
    const vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_FULLSCREEN_PASSES_PER_FRAME
    };

    // no eFreeDescriptorSet: sets are never freed one by one, the whole pool is reset (BeginFrame)
    const vk::DescriptorPoolCreateInfo poolCreateInfo{
        .maxSets = MAX_FULLSCREEN_PASSES_PER_FRAME,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };

    m_framePools.reserve(m_framesInFlight);
    for (std::uint32_t frameSlot = 0; frameSlot < m_framesInFlight; ++frameSlot)
        m_framePools.emplace_back(Device(), poolCreateInfo);
}

void Compositor::CreatePipelineLayout()
{
    const vk::PushConstantRange pushConstantRange{
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .offset = 0,
        .size = sizeof(PostProcess::PushConstants)
    };

    const vk::PipelineLayoutCreateInfo createInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*m_descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    m_pipelineLayout = vk::raii::PipelineLayout(Device(), createInfo);
}

vk::DescriptorSet Compositor::AllocateSourceSet(vk::ImageView source)
{
    const vk::DescriptorSetLayout layout = *m_descriptorSetLayout;
    const vk::DescriptorSetAllocateInfo allocateInfo{
        .descriptorPool = *m_framePools[m_frameSlot],
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };

    // A plain handle, not vk::raii::DescriptorSet: a RAII set frees itself on destruction, which needs
    // eFreeDescriptorSet on the pool. The pool owns it until the slot's next reset, so call through the device's
    // dispatcher (the same function table the RAII wrappers use).
    vk::DescriptorSet descriptorSet{};
    Vulkan::Utils::CheckResult(
        static_cast<vk::Result>(Device().getDispatcher()->vkAllocateDescriptorSets(
            *Device(),
            reinterpret_cast<const VkDescriptorSetAllocateInfo*>(&allocateInfo),
            reinterpret_cast<VkDescriptorSet*>(&descriptorSet))),
        "Compositor: descriptor pool of the frame slot is exhausted (too many fullscreen passes)");

    const vk::DescriptorImageInfo imageInfo{
        .sampler = *m_sampler,
        .imageView = source,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    const vk::WriteDescriptorSet write{
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo
    };
    Device().updateDescriptorSets(write, {});

    return descriptorSet;
}

const vk::raii::Device& Compositor::Device() const noexcept
{
    return m_context->GetDevice();
}
