/// @file Renderer.hpp
/// @brief Vulkan::Renderer: renders a Game::Scene to the window. The one rendering object the app talks to.
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Core/SurfaceProvider.hpp"
#include "Graphics/Vulkan/Passes/Compositor.hpp"
#include "Graphics/Vulkan/Core/Context.hpp"
#include "Graphics/Vulkan/Frame/RenderDevice.hpp"
#include "Graphics/Vulkan/Passes/DrawPass.hpp"
#include "Graphics/Vulkan/Passes/ImGuiPass.hpp"
#include "Graphics/Vulkan/Passes/EffectPass.hpp"
#include "Graphics/Vulkan/Passes/ImGuiPass.hpp"
#include "Graphics/Vulkan/Graph/GraphExecutor.hpp"
#include "Graphics/Vulkan/Resources/PipelineLibrary.hpp"
#include "Graphics/Vulkan/WSI/WSI.hpp"
#include "ResourceBridge/ResourceTransfer.hpp"
#include "Graphics/Render/RenderWorld.hpp"
#include "Graphics/Render/PreparedFrame.hpp"

#include <tracy/public/tracy/Tracy.hpp>


struct ImDrawData;

namespace Droplet::Graphics::Vulkan
{
    /**
     * @brief Owns everything on the GPU side; Render(world) runs Prepare, BeginFrame, the render graph and EndFrame
     * @pre CameraRenderData has been converted to Render::RenderWorld and resources used by the scene or that are 
     * marked as missing by the GPU-stage are uploaded.
     *   1. World    the scene's CameraRenderData   -> Render::RenderWorld      by GUID, no GPU calls
     *   2. Provide  Bridge::RequiredAssetsOf      what the GPU lacks -> AssetUploads -> Upload()
     * @details One frame, by stage.
     *   1. Prepare  Render::PlanViews, Prepare   RenderWorld -> Render::PreparedFrame by handle,  placeholders for gaps
     *   2. Build    Vulkan::BuildFrameGraph    PreparedFrame -> RenderGraph::Graph<PassContext>
     *   3. Compile  RenderGraph::Compile       Graph -> Compiled (schedule, barriers, waits)
     *   4. Execute  Vulkan::GraphExecutor      Compiled -> command buffers -> submit and present
     */
    class Renderer
    {
    public:
        /**
         * @brief Creates and manages all relevant rendering interfaces.
         * @param surfaceProvider abstract type that wraps a platform window (SDL3, glfw, win32, etc.)
         */
        explicit Renderer(const ISurfaceProvider& surfaceProvider);

        /// @brief Destructor. Waits for the GPU to go idle before anything is destroyed
        ~Renderer() noexcept;

        // no copy, no move
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        /** 
         * @brief Renders and presents a render world
         * @param world A struct containing CameraRenderData, view settings, a present target id and time
         * @param overlay Optionally renders ImGui draw data on top of the rendered output
         * @details Any resources referred to that are not yet uploaded to the GPU will be rendered as placeholders 
         * until they are available.
         */
        void Render(const Render::RenderWorld& world, ImDrawData* overlay = nullptr);

#ifdef _EDITOR
        /// @brief Initialises ImGui's Vulkan backend. Called once @pre ImGui context exists
        void InitImGui();
        /// @brief Starts ImGui's Vulkan backend. Called per-frame before ImGui::NewFrame
        void NewImGuiFrame();
        /**
        * @brief Converts the images of an ImGui frame into GUIDs used for The images an ImGui frame shows by ImGuiTextureId (for Bridge::RequiredAssetsOf)
        */
        [[nodiscard]] static std::vector<GUID> ImGuiImages(const ImDrawData& overlay);
#endif

        /// @brief Uploads assets (see RenderDevice::Upload). Call between frames; the spans only have to live until
        ///        this returns.
        void Upload(const Bridge::ResourceUploads& uploads)
        {
            ZoneScoped;
            m_device.Upload(uploads);
        }

        /// @brief The assets on the GPU with their revisions; pass to Bridge::ProvideAssets.
        [[nodiscard]] Bridge::ResidentResources Residents() const { return m_device.Residents(); }

        /// @brief Recreate the swapchain after the next present
        void OnResize() { m_wsi.OnResize(); }
        /// @brief Render does nothing until OnRestore
        void OnMinimize() { m_wsi.OnMinimize(); }
        /// @brief The window is visible again.
        void OnRestore() { m_wsi.OnRestore(); }
        [[nodiscard]] bool IsMinimized() const noexcept { return m_wsi.IsMinimized(); }

        /// @brief false: the composite only clears the window instead of showing the presented view (editor mode,
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
        /// @pre Editor mode only
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
        std::unordered_set<std::uint64_t> m_reportedMeshes; ///< dynamic meshes reported as not drawable (once each)

        Render::PreparedFrame m_preparedFrame; ///< This frame, prepared; the graph's pass callbacks read it
        bool m_compositeScene{ true };         ///< SetCompositeScene

#ifdef _EDITOR
        std::optional<ImGuiPass> m_imguiPass;            ///< after InitImGui
        std::vector<Render::TargetHandle> m_overlayReads; ///< this frame's targets the overlay shows
#endif
    };
}
