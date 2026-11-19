/// @file Renderer.hpp
/// @brief Vulkan::Renderer: renders a Game::Scene to the window. The one rendering object the app talks to.
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Vulkan/Core/SurfaceProvider.hpp"
#include "Integration/Vulkan/Passes/Compositor.hpp"
#include "Integration/Vulkan/Core/Context.hpp"
#include "Integration/Vulkan/Frame/RenderDevice.hpp"
#include "Integration/Vulkan/Passes/DrawPass.hpp"
#include "Integration/Vulkan/Passes/ImGuiPass.hpp"
#include "Integration/Vulkan/Passes/EffectPass.hpp"
#include "Integration/Vulkan/Passes/ImGuiPass.hpp"
#include "Integration/Vulkan/Graph/GraphExecutor.hpp"
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"
#include "Integration/Vulkan/WSI/WSI.hpp"
#include "Integration/Assets/AssetTransfer.hpp"
#include "Integration/Render/RenderWorld.hpp"
#include "Integration/Render/PreparedFrame.hpp"

struct ImDrawData;

namespace Vulkan
{
    /**
     * @brief Owns everything on the GPU side; Render(world) runs Prepare, BeginFrame, the render graph and EndFrame.
     * @details One frame, by stage. The app does 1 and 2 (Bridge); Render does 3 to 6.
     *   1. Extract  Bridge::Extract            Scene -> Render::RenderWorld      by GUID, no GPU calls
     *   2. Provide  Bridge::ProvideAssets      RequiredAssetsOf(world) and Resident() -> AssetUploads -> Upload()
     *   3. Prepare  Vulkan::Prepare            RenderWorld -> Render::PreparedFrame      by handle, placeholders for gaps
     *   4. Build    Vulkan::BuildFrameGraph    PreparedFrame -> RenderGraph::Graph<PassContext>
     *   5. Compile  RenderGraph::Compile       Graph -> Compiled (schedule, barriers, waits)
     *   6. Execute  Vulkan::GraphExecutor      Compiled -> command buffers -> submit and present
     */
    class Renderer
    {
    public:
        /// @brief Creates the context, swapchain, compositor, effect pass and resource managers, and compiles the
        ///        shaders.
        /// @param surfaceProvider must outlive the scene renderer
        explicit Renderer(const ISurfaceProvider& surfaceProvider);

        /// @brief Waits for the GPU to go idle before anything is destroyed
        ~Renderer() noexcept;

        // no copy, no move
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        /// @brief Renders and presents `world`; does nothing while minimized. Assets it refers to that are not on the
        ///        GPU (yet) are drawn as placeholders.
        void Render(const Render::RenderWorld& world, ImDrawData* overlay = nullptr);

#ifdef _EDITOR
        // == ImGui == the engine side owns the context and the frame (ImGuiFrame); the renderer draws it

        /// @brief Initialises ImGui's Vulkan backend. @pre an ImGui context exists; call once
        void InitImGui();
        /// @brief The backend's per-frame start; call before ImGui::NewFrame
        void NewImGuiFrame();
        /// @brief An ImGui texture id that shows `image`, a render target (a camera's output) or a texture asset:
        ///        pass it to ImGui::Image. The id stays the same across frames; each frame it is pointed at the image's
        ///        current GPU copy, and the frame graph reads the targets the overlay shows. A target no view draws
        ///        this frame shows its last image; one never drawn, or a texture not on the GPU (yet), shows the
        ///        placeholder texture.
        [[nodiscard]] std::uint64_t ImGuiTexture(Guid image);
        /// @brief The images ImGuiTexture was asked for since the last NewImGuiFrame; pass to Bridge::RequiredAssetsOf
        ///        so the texture assets among them are uploaded
        [[nodiscard]] std::span<const Guid> ImGuiImages() const noexcept { return m_imguiImagesThisFrame; }
#endif

        /// @brief Uploads assets (see RenderDevice::Upload). Call between frames; the spans only have to live until
        ///        this returns.
        void Upload(const Assets::AssetUploads& uploads) { m_device.Upload(uploads); }

        /// @brief The assets on the GPU with their revisions; pass to Bridge::ProvideAssets.
        [[nodiscard]] Assets::ResidentAssets Resident() const { return m_device.Resident(); }

        /// @brief Recreate the swapchain after the next present
        void OnResize() { m_wsi.OnResize(); }
        /// @brief Render does nothing until OnRestore
        void OnMinimize() { m_wsi.OnMinimize(); }
        /// @brief The window is visible again.
        void OnRestore() { m_wsi.OnRestore(); }
        [[nodiscard]] bool IsMinimized() const noexcept { return m_wsi.IsMinimized(); }

        /// @brief False: the composite only clears the window instead of showing the presented view (editor mode,
        ///        where viewport windows show the cameras); true by default
        void SetCompositeScene(bool compositeScene) noexcept { m_compositeScene = compositeScene; }

        /// @brief Blocks until the GPU has finished all submitted work
        void WaitIdle() const{ m_vulkanContext.WaitIdle(); }

        /// @brief The last prepared frame: a draw list per view (for debugging and tools).
        [[nodiscard]] const Render::PreparedFrame& Prepared() const noexcept { return m_preparedFrame; }

    private:
        /// @brief Compiles the fullscreen and effect shaders (scene shaders, the default included, compile in Prepare).
        void CompileShaders();

        /// @brief Builds, compiles and executes this frame's render graph; a fallback graph clears the swapchain image
        ///        if it does not compile.
        void RecordFrameGraph(const Frame& frame, ImDrawData* overlay);
#ifdef _EDITOR
        /// @brief Points the overlay's render-target ids at this frame's images; returns the targets they show
        std::vector<Render::TargetHandle> ResolveImGuiTextures(ImDrawData& overlay, const Render::RenderWorld& world);
#endif

        /// @brief Physical image of an import: the frame's swapchain image or a render target.
        [[nodiscard]] PhysicalResource ResolveImport(const RenderGraph::ResourceInfo& resource, const Frame& frame) const;

    private:
        // dependency order top to bottom: the context is created first and destroyed last
        Context m_vulkanContext;
        vk::Format m_depthFormat{vk::Format::eUndefined};
        WSI m_wsi;               ///< Swapchain, acquire and present, frame slots
        /// Frame submitter, GPU resources, scene shaders and the deletion queue; flushed in ~Renderer, because the
        /// deletion queue can hold DrawPass's material sets, whose pool goes before the device
        RenderDevice m_device;
        Compositor m_compositor; ///< Fullscreen pipelines: fragment effects, effect-chain resolves, the composite
        EffectPass m_effectPass; ///< Compute pipeline of the compute effects
        GraphExecutor m_graphExecutor;

        // == Scene pass and pipelines ==
        DrawPass m_drawPass;               ///< Scene material interface and draw loop, shared by every view
        PipelineLibrary m_pipelineLibrary; ///< Pipelines by description; requested in Prepare

        Render::PreparedFrame m_preparedFrame; ///< This frame, prepared; the graph's pass callbacks read it
        bool m_compositeScene{ true };         ///< SetCompositeScene

#ifdef _EDITOR
        std::optional<ImGuiPass> m_imguiPass;            ///< after InitImGui; destroyed before the device
        std::vector<Guid> m_imguiImages;                 ///< the images of ImGuiTexture ids, by index
        std::vector<Guid> m_imguiImagesThisFrame;        ///< ImGuiImages
        std::vector<Render::TargetHandle> m_overlayReads; ///< this frame's targets the overlay shows
#endif
    };
}
