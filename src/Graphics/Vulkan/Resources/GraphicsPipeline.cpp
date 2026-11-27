/// @file GraphicsPipeline.cpp
/// @brief GraphicsPipelineDesc::Hash and graphics pipeline creation from a GraphicsPipelineDesc.

#include "Graphics/Vulkan/Resources/GraphicsPipeline.hpp"

#include "Graphics/Vulkan/Abstraction/Shader.hpp"

#include <array>
#include <bit>
#include <functional>
#include <stdexcept>

namespace
{
    void HashCombine(std::uint64_t& seed, std::uint64_t value)
    {
        seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
    }

    template <typename Bits>
    std::uint64_t F(vk::Flags<Bits> flags)
    {
        return static_cast<std::uint64_t>(static_cast<typename vk::Flags<Bits>::MaskType>(flags));
    }

    constexpr vk::ColorComponentFlags ALL_COMPONENTS =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendAttachmentState ToBlendState(Droplet::Graphics::Vulkan::BlendMode mode)
    {
        switch (mode)
        {
        case Droplet::Graphics::Vulkan::BlendMode::AlphaBlend:
            return {
                .blendEnable = vk::True,
                .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .alphaBlendOp = vk::BlendOp::eAdd,
                .colorWriteMask = ALL_COMPONENTS
            };
        case Droplet::Graphics::Vulkan::BlendMode::Additive:
            return {
                .blendEnable = vk::True,
                .srcColorBlendFactor = vk::BlendFactor::eOne,
                .dstColorBlendFactor = vk::BlendFactor::eOne,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                .dstAlphaBlendFactor = vk::BlendFactor::eOne,
                .alphaBlendOp = vk::BlendOp::eAdd,
                .colorWriteMask = ALL_COMPONENTS
            };
        case Droplet::Graphics::Vulkan::BlendMode::Opaque:
        default:
            return {.blendEnable = vk::False, .colorWriteMask = ALL_COMPONENTS};
        }
    }
}

namespace Droplet::Graphics::Vulkan
{
    std::uint64_t GraphicsPipelineDesc::Hash() const
    {
        std::uint64_t seed = 0;
        HashCombine(seed, shaders.vertex);
        HashCombine(seed, shaders.tessControl);
        HashCombine(seed, shaders.tessEvaluation);
        HashCombine(seed, shaders.geometry);
        HashCombine(seed, shaders.fragment);
        HashCombine(seed, reinterpret_cast<std::uint64_t>(static_cast<VkPipelineLayout>(layout)));

        for (const vk::VertexInputBindingDescription& binding : vertexLayout.bindings)
        {
            HashCombine(seed, binding.binding);
            HashCombine(seed, binding.stride);
            HashCombine(seed, static_cast<std::uint64_t>(binding.inputRate));
        }
        for (const vk::VertexInputAttributeDescription& attribute : vertexLayout.attributes)
        {
            HashCombine(seed, attribute.location);
            HashCombine(seed, attribute.binding);
            HashCombine(seed, static_cast<std::uint64_t>(attribute.format));
            HashCombine(seed, attribute.offset);
        }

        HashCombine(seed, static_cast<std::uint64_t>(topology));
        HashCombine(seed, static_cast<std::uint64_t>(raster.polygonMode));
        HashCombine(seed, F(raster.cullMode));
        HashCombine(seed, static_cast<std::uint64_t>(raster.frontFace));
        HashCombine(seed, std::bit_cast<std::uint32_t>(raster.lineWidth));
        HashCombine(seed, depth.testEnable ? 1u : 0u);
        HashCombine(seed, depth.writeEnable ? 1u : 0u);
        HashCombine(seed, static_cast<std::uint64_t>(depth.compareOp));

        for (const vk::Format format : colorFormats)
            HashCombine(seed, static_cast<std::uint64_t>(format));
        for (const BlendMode mode : colorBlend)
            HashCombine(seed, static_cast<std::uint64_t>(mode));
        HashCombine(seed, static_cast<std::uint64_t>(depthFormat));
        HashCombine(seed, static_cast<std::uint64_t>(samples));
        return seed;
    }

    GraphicsPipeline::GraphicsPipeline(
        const vk::raii::Device& device,
        const GraphicsPipelineDesc& desc,
        const Stages& stages,
        const vk::raii::PipelineCache* cache)
        : m_layout(desc.layout)
    {
        if (!stages.vertex)
            throw std::invalid_argument("GraphicsPipeline: no vertex shader");
        if ((stages.tessControl == nullptr) != (stages.tessEvaluation == nullptr))
            throw std::invalid_argument("GraphicsPipeline: tessellation needs both a control and an evaluation shader");

        // in the order Vulkan runs them; each must be of its stage's type
        const std::array<std::pair<const Shader*, vk::ShaderStageFlagBits>, 5> ordered{ {
            { stages.vertex, vk::ShaderStageFlagBits::eVertex },
            { stages.tessControl, vk::ShaderStageFlagBits::eTessellationControl },
            { stages.tessEvaluation, vk::ShaderStageFlagBits::eTessellationEvaluation },
            { stages.geometry, vk::ShaderStageFlagBits::eGeometry },
            { stages.fragment, vk::ShaderStageFlagBits::eFragment },
        } };
        std::array<vk::PipelineShaderStageCreateInfo, 5> stageInfos{};
        std::uint32_t stageCount = 0;
        for (const auto& [shader, stage] : ordered)
        {
            if (!shader)
            {
                continue;
            }
            if (shader->Stage() != stage)
            {
                throw std::invalid_argument("GraphicsPipeline: a shader is of the wrong stage");
            }
            stageInfos[stageCount++] = shader->StageInfo();
        }
        if (!desc.layout)
            throw std::invalid_argument("GraphicsPipeline: pipeline layout is null");
        if (desc.colorBlend.size() != desc.colorFormats.size())
            throw std::invalid_argument("GraphicsPipeline: colorBlend and colorFormats size mismatch");


        constexpr std::array dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        const vk::PipelineDynamicStateCreateInfo dynamicInfo{
            .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        const auto& vl = desc.vertexLayout;
        const vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount = static_cast<std::uint32_t>(vl.bindings.size()),
            .pVertexBindingDescriptions = vl.bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vl.attributes.size()),
            .pVertexAttributeDescriptions = vl.attributes.data()
        };

        const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
            .topology = desc.topology,
            .primitiveRestartEnable = vk::False
        };

        const vk::PipelineViewportStateCreateInfo viewportInfo{
            .viewportCount = 1,
            .scissorCount = 1
        };

        const vk::PipelineRasterizationStateCreateInfo rasterInfo{
            .polygonMode = desc.raster.polygonMode,
            .cullMode = desc.raster.cullMode,
            .frontFace = desc.raster.frontFace,
            .lineWidth = desc.raster.lineWidth
        };

        const vk::PipelineMultisampleStateCreateInfo multisampleInfo{
            .rasterizationSamples = desc.samples
        };

        const vk::PipelineDepthStencilStateCreateInfo depthInfo{
            .depthTestEnable = desc.depth.testEnable ? vk::True : vk::False,
            .depthWriteEnable = desc.depth.writeEnable ? vk::True : vk::False,
            .depthCompareOp = desc.depth.compareOp
        };
        const bool hasDepth = desc.depthFormat != vk::Format::eUndefined;

        std::vector<vk::PipelineColorBlendAttachmentState> blendStates;
        blendStates.reserve(desc.colorBlend.size());
        for (const BlendMode mode : desc.colorBlend)
            blendStates.push_back(ToBlendState(mode));

        const vk::PipelineColorBlendStateCreateInfo colorBlendInfo{
            .logicOpEnable = vk::False,
            .attachmentCount = static_cast<std::uint32_t>(blendStates.size()),
            .pAttachments = blendStates.data()
        };

        // triangle patches: a tessellation pipeline's topology is patch list with 3 control points per triangle
        constexpr std::uint32_t TRIANGLE_PATCH_CONTROL_POINTS = 3;
        const vk::PipelineTessellationStateCreateInfo tessellationInfo{ .patchControlPoints = TRIANGLE_PATCH_CONTROL_POINTS };
        const bool tessellation = stages.tessControl != nullptr;

        const vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> chain{
            {
                .stageCount = stageCount,
                .pStages = stageInfos.data(),
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssemblyInfo,
                .pTessellationState = tessellation ? &tessellationInfo : nullptr,
                .pViewportState = &viewportInfo,
                .pRasterizationState = &rasterInfo,
                .pMultisampleState = &multisampleInfo,
                .pDepthStencilState = hasDepth ? &depthInfo : nullptr,
                .pColorBlendState = &colorBlendInfo,
                .pDynamicState = &dynamicInfo,
                .layout = desc.layout,
                .renderPass = nullptr
            },
            {
                .viewMask = 0,
                .colorAttachmentCount = static_cast<std::uint32_t>(desc.colorFormats.size()),
                .pColorAttachmentFormats = desc.colorFormats.data(),
                .depthAttachmentFormat = desc.depthFormat,
                .stencilAttachmentFormat = vk::Format::eUndefined
            }
        };

        const auto& createInfo = chain.get<vk::GraphicsPipelineCreateInfo>();
        m_pipeline = cache
                         ? vk::raii::Pipeline(device, *cache, createInfo)
                         : vk::raii::Pipeline(device, nullptr, createInfo);
    }
}
