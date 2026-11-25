/// @file ImGuiPass.hpp
/// @brief Vulkan::ImGuiPass: the renderer's side of ImGui, the Vulkan backend. Editor builds only (_EDITOR).
#pragma once
#ifdef _EDITOR

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

struct ImDrawData;

namespace Droplet::Graphics::Vulkan
{
    class Context;

    /// @brief Draws ImGui's draw data with imgui_impl_vulkan (dynamic rendering, no depth).
    /// @details The ImGui context and frame belong to the engine side (ImGuiFrame); this only owns the backend.
    ///          Images the windows show get a descriptor set per frame slot, freed when the slot comes round again.
    class ImGuiPass
    {
    public:
        /// @brief Initialises the backend. @pre an ImGui context exists
        ImGuiPass(const Context& context, vk::Format colorFormat, std::uint32_t framesInFlight);
        /// @brief Shuts the backend down. @pre the GPU is idle; the ImGui context still exists
        ~ImGuiPass();

        // no copy no move
        ImGuiPass(const ImGuiPass&) = delete;
        ImGuiPass& operator=(const ImGuiPass&) = delete;
        ImGuiPass(ImGuiPass&&) = delete;
        ImGuiPass& operator=(ImGuiPass&&) = delete;

        /// @brief The backend's per-frame start; called before ImGui::NewFrame
        void NewFrame();

        /// @brief Rebuilds the pipeline for a new swapchain format. @pre the GPU is idle
        void SetColorFormat(vk::Format colorFormat);
        [[nodiscard]] vk::Format ColorFormat() const noexcept { return m_colorFormat; }

        /// @brief Starts a frame in `frameSlot`: frees the descriptor sets the slot's previous frame used.
        /// @pre the GPU is done with the slot's previous frame
        void BeginFrame(std::uint32_t frameSlot);

        /// @brief Uploads the textures ImGui created or changed (its font atlas), before the frame graph. The backend
        ///        submits on the graphics queue and waits for it, so this stalls, but only when a texture changed.
        void UpdateTextures(ImDrawData& drawData);

        /// @brief This frame's descriptor set for `view`, created the first time it is asked for in the frame
        [[nodiscard]] vk::DescriptorSet SetFor(vk::ImageView view, vk::Sampler sampler);

        /// @brief Draws `drawData` into the open render pass instance (one colour attachment of ColorFormat())
        void Record(vk::CommandBuffer commandBuffer, ImDrawData& drawData);

    private:
        const Context* m_context; ///< Not owned; outlives this.
        vk::Format m_colorFormat;
        VkFormat m_attachmentFormat; ///< what the backend's pipeline info points at; lives as long as the backend
        std::uint32_t m_frameSlot{ 0 };
        std::vector<std::vector<vk::DescriptorSet>> m_frameSets;          ///< per frame slot: sets of SetFor
        std::unordered_map<VkImageView, vk::DescriptorSet> m_setsThisFrame; ///< this frame's, by view
    };
}

#endif
