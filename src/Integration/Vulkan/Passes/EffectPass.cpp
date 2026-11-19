/**
 * @file EffectPass.cpp
 * @brief Vulkan::EffectPass implementation: the post-process compute pipelines and their per-frame-slot descriptor
 *        pools. The passes themselves are render-graph passes on the compute queue (FrameGraphBuilder.cpp).
 */
#include "Integration/Vulkan/Passes/EffectPass.hpp"

#include <array>
#include <cassert>
#include <vector>

#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Core/VulkanUtils.hpp"

using namespace Vulkan;

EffectPass::EffectPass(const Context& context, std::uint32_t framesInFlight)
    : m_context{&context},
      m_framesInFlight{framesInFlight}
{
    CreateDescriptorSetLayout();
    CreateSampler();
    CreateDescriptorPools();
    CreatePipelineLayout();
}

void EffectPass::CreatePipelines(const ShaderCode& shaderCode)
{
    // nothing in a pipeline depends on the images' size, so one per entry point serves every view
    const auto makePipeline = [this](const std::vector<std::uint32_t>& spirv, const char* entryPoint)
    {
        const vk::raii::ShaderModule shaderModule{Device(), vk::ShaderModuleCreateInfo{
            .codeSize = spirv.size() * sizeof(std::uint32_t), // bytes
            .pCode = spirv.data()
        }};
        const vk::ComputePipelineCreateInfo pipelineCreateInfo{
            .stage = {
                .stage = vk::ShaderStageFlagBits::eCompute,
                .module = *shaderModule,
                .pName = entryPoint
            },
            .layout = *m_pipelineLayout
        };
        return vk::raii::Pipeline{Device(), nullptr, pipelineCreateInfo};
    };

    m_imageEffectPipeline = makePipeline(shaderCode.imageEffect, "cMain");
    m_measureLuminancePipeline = makePipeline(shaderCode.measureLuminance, "cMeasureLuminance");
    m_applyExposurePipeline = makePipeline(shaderCode.applyExposure, "cApplyExposure");
}

void EffectPass::BeginFrame(std::uint32_t frameSlot)
{
    assert(frameSlot < m_framesInFlight);
    m_frameSlot = frameSlot;

    // every set allocated from this pool belonged to the frame that last used the slot, which the GPU has finished
    m_framePools[frameSlot].reset();
}

void EffectPass::Record(
    vk::CommandBuffer commandBuffer,
    vk::ImageView source,
    vk::ImageView destination,
    vk::Buffer stepBuffer,
    vk::Extent2D extent,
    const PostProcess::PushConstants& pushConstants)
{
    const bool measuring = pushConstants.effect == PostProcess::Effect::MeasureLuminance;
    const vk::Pipeline pipeline = measuring ? *m_measureLuminancePipeline
                                : pushConstants.effect == PostProcess::Effect::ApplyExposure ? *m_applyExposurePipeline
                                : *m_imageEffectPipeline;
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout,
        0,
        AllocateStepSet(source, destination, stepBuffer),
        nullptr);

    commandBuffer.pushConstants(
        *m_pipelineLayout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        sizeof(PostProcess::PushConstants),
        &pushConstants);

    // measuring is one work group whatever the size; otherwise one work group covers WORK_GROUP_SIZE x
    // WORK_GROUP_SIZE pixels and the shader skips the ones past the edge
    if (measuring)
    {
        commandBuffer.dispatch(1, 1, 1);
        return;
    }
    commandBuffer.dispatch(
        (extent.width + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE,
        (extent.height + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE,
        1);
}

void EffectPass::CreateDescriptorSetLayout()
{
    constexpr std::array bindings{
        vk::DescriptorSetLayoutBinding{
            .binding = 0, // the image the effect reads
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute
        },
        vk::DescriptorSetLayoutBinding{
            .binding = 1, // the image the effect writes
            .descriptorType = vk::DescriptorType::eStorageImage,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute
        },
        vk::DescriptorSetLayoutBinding{
            .binding = 2, // the effect's step buffer
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute
        }
    };

    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<std::uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    m_descriptorSetLayout = vk::raii::DescriptorSetLayout(Device(), createInfo);
}

void EffectPass::CreateSampler()
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

void EffectPass::CreateDescriptorPools()
{
    constexpr std::array poolSizes{
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = MAX_EFFECT_PASSES_PER_FRAME
        },
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageImage,
            .descriptorCount = MAX_EFFECT_PASSES_PER_FRAME
        },
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eStorageBuffer,
            .descriptorCount = MAX_EFFECT_PASSES_PER_FRAME
        }
    };

    // no eFreeDescriptorSet: sets are never freed one by one, the whole pool is reset (BeginFrame)
    const vk::DescriptorPoolCreateInfo poolCreateInfo{
        .maxSets = MAX_EFFECT_PASSES_PER_FRAME,
        .poolSizeCount = static_cast<std::uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    m_framePools.reserve(m_framesInFlight);
    for (std::uint32_t frameSlot = 0; frameSlot < m_framesInFlight; ++frameSlot)
        m_framePools.emplace_back(Device(), poolCreateInfo);
}

void EffectPass::CreatePipelineLayout()
{
    const vk::PushConstantRange pushConstantRange{
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
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

vk::DescriptorSet EffectPass::AllocateStepSet(
    vk::ImageView source, vk::ImageView destination, vk::Buffer stepBuffer)
{
    const vk::DescriptorSetLayout layout = *m_descriptorSetLayout;
    const vk::DescriptorSetAllocateInfo allocateInfo{
        .descriptorPool = *m_framePools[m_frameSlot],
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };

    // A plain handle, not vk::raii::DescriptorSet: the pool owns the set until the slot's next reset (see
    // Compositor::AllocateSourceSet).
    vk::DescriptorSet descriptorSet{};
    Vulkan::Utils::CheckResult(
        static_cast<vk::Result>(Device().getDispatcher()->vkAllocateDescriptorSets(
            *Device(),
            reinterpret_cast<const VkDescriptorSetAllocateInfo*>(&allocateInfo),
            reinterpret_cast<VkDescriptorSet*>(&descriptorSet))),
        "EffectPass: descriptor pool of the frame slot is exhausted (too many effect passes)");

    const vk::DescriptorImageInfo sourceInfo{
        .sampler = *m_sampler,
        .imageView = source,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    const vk::DescriptorImageInfo destinationInfo{
        .imageView = destination,
        .imageLayout = vk::ImageLayout::eGeneral
    };
    const vk::DescriptorBufferInfo stepBufferInfo{
        .buffer = stepBuffer,
        .offset = 0,
        .range = vk::WholeSize
    };

    // only what the step uses: an entry point that does not use a binding may leave it unwritten
    std::vector<vk::WriteDescriptorSet> writes{
        vk::WriteDescriptorSet{
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &sourceInfo
        }
    };
    if (destination)
    {
        writes.push_back({
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageImage,
            .pImageInfo = &destinationInfo
        });
    }
    if (stepBuffer)
    {
        writes.push_back({
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &stepBufferInfo
        });
    }
    Device().updateDescriptorSets(writes, {});

    return descriptorSet;
}

const vk::raii::Device& EffectPass::Device() const noexcept
{
    return m_context->GetDevice();
}
