/// @file ImGuiPass.cpp
/// @brief Vulkan::ImGuiPass implementation.
#ifdef _EDITOR
#include "Integration/Vulkan/Passes/ImGuiPass.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>

#include "Integration/Vulkan/Core/Context.hpp"

namespace
{
    /// @brief Distinct images the windows can show in one frame (render targets), on top of ImGui's own textures
    constexpr std::uint32_t MAX_IMAGES_PER_FRAME = 32;

    void CheckVkResult(VkResult result)
    {
        if (result != VK_SUCCESS)
            throw std::runtime_error(std::format("imgui_impl_vulkan: VkResult {}", static_cast<int>(result)));
    }

    /// @brief How the backend builds its pipeline: dynamic rendering, one colour attachment, no depth
    ImGui_ImplVulkan_PipelineInfo PipelineInfo(const VkFormat* colorFormat)
    {
        ImGui_ImplVulkan_PipelineInfo info{};
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        info.PipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormat;
        info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        return info;
    }
}

namespace Vulkan
{
    ImGuiPass::ImGuiPass(const Context& context, vk::Format colorFormat, std::uint32_t framesInFlight)
        : m_context(&context),
          m_colorFormat(colorFormat),
          m_attachmentFormat(static_cast<VkFormat>(colorFormat)),
          m_frameSets(framesInFlight)
    {
        ImGui_ImplVulkan_InitInfo info{};
        info.ApiVersion = VK_API_VERSION_1_3;
        info.Instance = *context.GetInstance();
        info.PhysicalDevice = *context.GetPhysicalDevice();
        info.Device = *context.GetDevice();
        info.QueueFamily = context.GetGraphicsPresentQueueFamily();
        info.Queue = *context.GetGraphicsQueue();
        info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE + MAX_IMAGES_PER_FRAME * framesInFlight;
        // the backend cycles its vertex and index buffers through ImageCount frames: one per frame in flight
        info.MinImageCount = 2;
        info.ImageCount = std::max(2u, framesInFlight);
        info.UseDynamicRendering = true;
        info.PipelineInfoMain = PipelineInfo(&m_attachmentFormat);
        info.CheckVkResultFn = CheckVkResult;
        if (!ImGui_ImplVulkan_Init(&info))
            throw std::runtime_error("ImGuiPass: ImGui_ImplVulkan_Init failed");
    }

    ImGuiPass::~ImGuiPass()
    {
        for (std::vector<vk::DescriptorSet>& sets : m_frameSets)
        {
            for (const vk::DescriptorSet set : sets)
                ImGui_ImplVulkan_RemoveTexture(set);
        }
        ImGui_ImplVulkan_Shutdown();
    }

    void ImGuiPass::NewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void ImGuiPass::SetColorFormat(vk::Format colorFormat)
    {
        if (colorFormat == m_colorFormat)
            return;
        m_colorFormat = colorFormat;
        m_attachmentFormat = static_cast<VkFormat>(colorFormat);
        const ImGui_ImplVulkan_PipelineInfo info = PipelineInfo(&m_attachmentFormat);
        ImGui_ImplVulkan_CreateMainPipeline(&info);
    }

    void ImGuiPass::BeginFrame(std::uint32_t frameSlot)
    {
        m_frameSlot = frameSlot;
        for (const vk::DescriptorSet set : m_frameSets[frameSlot])
            ImGui_ImplVulkan_RemoveTexture(set);
        m_frameSets[frameSlot].clear();
        m_setsThisFrame.clear();
    }

    void ImGuiPass::UpdateTextures(ImDrawData& drawData)
    {
        if (drawData.Textures)
        {
            for (ImTextureData* texture : *drawData.Textures)
            {
                if (texture->Status != ImTextureStatus_OK)
                    ImGui_ImplVulkan_UpdateTexture(texture);
            }
        }
        drawData.Textures = nullptr; // done here, not inside the graph pass
    }

    vk::DescriptorSet ImGuiPass::SetFor(vk::ImageView view, vk::Sampler sampler)
    {
        if (const auto setIt = m_setsThisFrame.find(static_cast<VkImageView>(view)); setIt != m_setsThisFrame.end())
            return setIt->second;
        const vk::DescriptorSet set = ImGui_ImplVulkan_AddTexture(
            static_cast<VkSampler>(sampler), static_cast<VkImageView>(view), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_frameSets[m_frameSlot].push_back(set);
        m_setsThisFrame.emplace(static_cast<VkImageView>(view), set);
        return set;
    }

    void ImGuiPass::Record(vk::CommandBuffer commandBuffer, ImDrawData& drawData)
    {
        ImGui_ImplVulkan_RenderDrawData(&drawData, static_cast<VkCommandBuffer>(commandBuffer));
    }
}
#endif
