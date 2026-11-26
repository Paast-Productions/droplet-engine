/**
 * @file Renderer.cpp
 * @brief Vulkan::Renderer implementation: the frame sequence, shader compilation, draw passes, and the render
 *        graph including post-processing and the composite into the swapchain image.
 */
#include "Graphics/Vulkan/Frame/Renderer.hpp"

#include <tracy/public/tracy/Tracy.hpp>


#include <algorithm>
#include <array>
#include <exception>
#include <format>
#include <print>
#include <stdexcept>

#include "Common/RenderData.hpp"
#include "Graphics/Render/ViewPlan.hpp"
#include "Graphics/Vulkan/Core/VulkanUtils.hpp"
#include "Graphics/Vulkan/Passes/PostProcessShader.hpp"
#include "Graphics/Vulkan/Frame/Prepare.hpp"
#include "Graphics/Vulkan/Graph/FrameGraphBuilder.hpp"
#include "Graphics/Vulkan/Graph/RenderGraphVk.hpp"
#include "Resource/IO/ShaderCompiler.hpp"

#ifdef _EDITOR
#include <ImGui/imgui.h>
#endif

using namespace Droplet;

namespace
{
    
    constexpr auto FULLSCREEN_SHADER_PATH = ROOT_PATH "/Assets/Shaders/fullscreen.vs.slang"; ///< fullscreen-triangle vertex shader
    constexpr auto POST_PROCESSING_SHADER_PATH = ROOT_PATH "/Assets/Shaders/postprocess.fs.slang"; ///< post-process fragment shader
    constexpr auto EFFECT_SHADER_PATH = ROOT_PATH "/Assets/Shaders/postprocess.cs.slang"; ///< post-process compute shader
    constexpr auto SCENE_SHADER_PATH = ROOT_PATH "/Assets/Shaders/scene.slang"; ///< scene vertex + fragment shader

    /// The renderer's own default scene shader, drawn with until the default pair production names is ready
    constexpr GUID BUILT_IN_SCENE_SHADER = GuidFromName("shader/built-in-scene");
    const Common::ShaderDesc BUILT_IN_SCENE_SHADER_ASSET{
        .moduleName = "scene",
        .sourcePath = SCENE_SHADER_PATH,
        .vertexEntry = "vMain",
        .fragmentEntry = "fMain"
    };

    /// How an acquired swapchain image was "last used": the acquire semaphore is waited on at colour-attachment output,
    /// so the first barrier (from UNDEFINED, contents discarded) must start at that stage, or the layout transition
    /// could run before the presentation engine has released the image.
    constexpr Graphics::Vulkan::ImageState SWAPCHAIN_ACQUIRE_USE{
        .layout = vk::ImageLayout::eUndefined,
        .accessMask = {},
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput
    };

    constexpr vk::ImageSubresourceRange COLOR_RANGE{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
}

using namespace Droplet::Graphics::Vulkan;

Renderer::Renderer(const ISurfaceProvider& surfaceProvider)
    : m_vulkanContext(surfaceProvider, ContextCreateInfo{}),
      // depth images are per-view render-graph transients; only their format is chosen here, once
      m_depthFormat(Vulkan::Utils::ChooseDepthFormat(m_vulkanContext.GetPhysicalDevice())),
      m_wsi(m_vulkanContext),
      // frame submitter (one timeline per queue, command pools per frame slot and queue), GPU resources with their
      // placeholders, scene shaders
      m_device(m_vulkanContext, WSI::FramesInFlight()),
      m_compositor(m_vulkanContext, WSI::FramesInFlight()),
      m_effectPass(m_vulkanContext, WSI::FramesInFlight()),
      m_graphExecutor(m_vulkanContext, m_device),
      m_drawPass(m_vulkanContext, m_device, DrawPassCreateInfo{.framesInFlight = WSI::FramesInFlight()}),
      m_pipelineLibrary(m_vulkanContext.GetDevice(), m_device.Shaders())
{
    m_device.Shaders().SetBuiltInDefault(BUILT_IN_SCENE_SHADER, BUILT_IN_SCENE_SHADER_ASSET);
    CompileShaders();
}

Renderer::~Renderer() noexcept
{
    // the members are destroyed after this body; the GPU must be done with all of them
    try
    {
        WaitIdle();
        m_device.FlushDeletions(); // before DrawPass's descriptor pool goes
    }
    catch (const std::exception& exception)
    {
        std::println("Vulkan device wait during shutdown failed: {}", exception.what());
    }
    catch (...)
    {
        std::println("Vulkan device wait during shutdown failed.");
    }
}

void Renderer::Render(const Render::RenderWorld& world, [[maybe_unused]] ImDrawData* overlay)
{
    ZoneScopedC(tracy::Color::LightGoldenrodYellow);
    
    if (m_wsi.IsMinimized())
        return;

    m_device.BeginFrame(); // what Prepare resolves counts as used this frame

    // 1. prepare: which cameras draw and what each reads, then the world (by GUID) becomes a draw list per drawn
    //    camera (by GPU handle, or a placeholder). From here on nothing looks at the world
    const std::vector<Render::ViewPlan> plans = Render::PlanViews(world);

    // kept dynamic meshes the GPU lacks or holds an older revision of, before anything is resolved
    for (std::size_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
    {
        if (!plans[cameraIndex].drawn)
            continue;
        for (const Common::RenderData& object : world.cameras[cameraIndex].renderData)
        {
            if (object.dynamicMesh && object.dynamicMesh->id != Droplet::C_INVALID_GUID)
                (void)m_device.Resources().UploadDynamicMesh(*object.dynamicMesh);
        }
    }

    m_preparedFrame = Prepare(world, plans, PrepareContext{
                                  .gpu = m_device.Resources(),
                                  .shaders = m_device.Shaders(),
                                  .pipelines = m_pipelineLibrary,
                                  .scenePass = m_drawPass,
                                  .depthFormat = m_depthFormat,
                                  .reportedMeshes = m_reportedMeshes
                              });

    // 2. wait until the GPU has finished the frame that last used the next frame slot, on every queue (its command
    //    buffers are freed, its acquire semaphore is free), then acquire a swapchain image; nothing to draw if the
    //    swapchain was out of date
    m_device.WaitForFrameSlot(m_wsi.FrameSlot());
    const std::optional<Frame> frame = m_wsi.BeginFrame();
    if (!frame)
    {
        // no frame available
        m_device.EndFrame();
        return;
    }

    // the slot's descriptor pools are free now (the submitter waited for the slot on every queue)
    m_compositor.BeginFrame(frame->frameSlot);
    m_effectPass.BeginFrame(frame->frameSlot);
    // the texture array this frame binds holds every texture and target Prepare gave an index
    m_device.Textures().BeginFrame(frame->frameSlot, m_device.Deletions(), m_device.Submitter().FrameRetireValues());
    m_drawPass.BeginFrame(frame->frameSlot, m_preparedFrame);

#ifdef _EDITOR
    // the editor's ImGui: its textures uploaded before the graph, its render-target images resolved for this frame
    if (!m_imguiPass)
        overlay = nullptr;
    m_overlayReads.clear();
    if (overlay)
    {
        if (frame->swapchainImage.format != m_imguiPass->ColorFormat())
        {
            WaitIdle(); // the old pipeline may still be in use
            m_imguiPass->SetColorFormat(frame->swapchainImage.format);
        }
        m_imguiPass->BeginFrame(frame->frameSlot);
        m_imguiPass->UpdateTextures(*overlay);
        m_overlayReads = ResolveImGuiTextures(*overlay, world);
    }
#endif

    // 3-4. the render graph: views, depth transients, effect chains (fragment passes, compute passes on the compute
    //      queue), history copies on the transfer queue, composite into the swapchain; recorded and submitted by the
    //      frame submitter
    RecordFrameGraph(*frame, overlay);

    // 5. present; the swapchain may be recreated
    m_wsi.EndFrame(*frame);

    // 6. resources nothing asked for in a while are released; what the GPU is done with is destroyed
    m_device.EndFrame();
    FrameMark;
}

void Renderer::CompileShaders()
{
    const Droplet::IO::ShaderCompiler& shaderCompiler = m_device.Shaders().Compiler();

    // fullscreen pipelines for fragment effects, effect-chain resolves and the composite (one per target format,
    // built on first use)
    const auto vertexSpirv = shaderCompiler.CompileSpirv(
        "fullscreen",
        FULLSCREEN_SHADER_PATH,
        "vMain");
    const auto fragmentSpirv = shaderCompiler.CompileSpirv(
        "postprocess",
        POST_PROCESSING_SHADER_PATH,
        "fMain");
    m_compositor.CreateShaderModules(vertexSpirv, fragmentSpirv);

    // compute effects: one pipeline per entry point
    m_effectPass.CreatePipelines(EffectPass::ShaderCode{
        .imageEffect = shaderCompiler.CompileSpirv("postprocess_compute", EFFECT_SHADER_PATH, "cMain"),
        .measureLuminance = shaderCompiler.CompileSpirv("postprocess_compute", EFFECT_SHADER_PATH, "cMeasureLuminance"),
        .applyExposure = shaderCompiler.CompileSpirv("postprocess_compute", EFFECT_SHADER_PATH, "cApplyExposure")
    });
}

void Renderer::RecordFrameGraph(const Frame& frame, [[maybe_unused]] ImDrawData* overlay)
{
    ZoneScoped
    
    FrameGraphInputs inputs{
        .currentState = [this](Render::TargetHandle handle)
        {
            // a target no graph has used yet: undefined, unless it was cleared when it was created (a history), then
            // it waits in ShaderReadOnly on the graphics queue (the clear has finished: the upload waited for it)
            RenderGraph::ResourceState state = m_graphExecutor.ImportState(Vulkan::TargetKey(handle));
            const GpuRenderTarget* target = m_device.Resources().Find(handle);
            if (state.usage == RenderGraph::Usage::None && target && target->clearedOnCreation)
                state = {.usage = RenderGraph::Usage::SampledRead, .queue = RenderGraph::Queue::Graphics, .value = 0};
            return state;
        },
        // nothing outside the graph displays a target: the composite reads the presented view
        .isDisplayed = nullptr,
        .depthFormat = static_cast<std::uint32_t>(m_depthFormat),
        .targetFormat = [this](Render::TargetHandle handle)
        {
            const GpuRenderTarget* target = m_device.Resources().Find(handle);
            return static_cast<std::uint32_t>(target ? target->target.Format() : vk::Format::eUndefined);
        },
        .effectFormat = static_cast<std::uint32_t>(EffectPass::IMAGE_FORMAT),
        .drawView = [this](PassContext& ctx, const Render::DrawList& view,
                           const Vulkan::ViewTargets& targets)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Draw view"};
            if (!ctx.resources)
                return;

            const vk::Extent2D extent = ctx.resources->Extent(targets.color);

            const vk::RenderingAttachmentInfo color{
                .imageView = ctx.resources->View(targets.color),
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .clearValue = vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32(
                    {view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a}))
            };

            // cleared here and not needed after the pass so don't store
            vk::RenderingAttachmentInfo depth = {};
            if (targets.depth.IsValid())
            {
                depth = vk::RenderingAttachmentInfo{
                    .imageView = ctx.resources->View(targets.depth),
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eDontCare,
                    .clearValue = vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{
                        .depth = 1.0f,
                        .stencil = 0
                    })
                };
            }

            ctx.cmd.beginRendering(vk::RenderingInfo{
                .renderArea = {.offset = {0, 0}, .extent = extent},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color,
                .pDepthAttachment = targets.depth.IsValid() ? &depth : nullptr
            });
            // All: the graph guarantees any target this view samples was written earlier in the frame
            const auto viewIndex = static_cast<std::uint32_t>(&view - m_preparedFrame.views.data());
            // a list of m_preparedFrame
            m_drawPass.Record(ctx.cmd, extent, view, viewIndex, m_pipelineLibrary, DrawFilter::All);
            ctx.cmd.endRendering();
        },
        .copyTarget = [this](PassContext& ctx, Render::TargetHandle srcHandle, Render::TargetHandle dstHandle)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Copy target"};
            const GpuRenderTarget* source = m_device.Resources().Find(srcHandle);
            const GpuRenderTarget* destination = m_device.Resources().Find(dstHandle);
            if (!source || !destination)
                return;

            const vk::Extent2D extent = source->target.Extent();
            constexpr vk::ImageSubresourceLayers colorLayer{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            const vk::ImageCopy region{
                .srcSubresource = colorLayer,
                .dstSubresource = colorLayer,
                .extent = vk::Extent3D{
                    .width = extent.width,
                    .height = extent.height,
                    .depth = 1
                },
            };

            // graph has already transitioned source to TransferSrc and destination to TransferDst.  whole-image
            // copy, so it is valid whatever the transfer queue family's minImageTransferGranularity is.
            ctx.cmd.copyImage(
                source->target.GetImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                destination->target.GetImage(),
                vk::ImageLayout::eTransferDstOptimal,
                region);
        },
        .fragmentEffect = [this](PassContext& ctx, const Render::DrawList& view,
                                 const EffectStep& step,
                                 RenderGraph::ResourceId source, RenderGraph::ResourceId destination)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Fragment effect"};
            // `destination` has the format of the view's target: the pipeline for that format
            const GpuRenderTarget* target = m_device.Resources().Find(view.target);
            if (!target)
                return;
            m_compositor.RecordFullscreen(
                ctx.cmd,
                ctx.resources->View(source),
                ctx.resources->View(destination),
                target->target.Format(),
                ctx.resources->Extent(destination),
                {.effect = step.effect, .amount = step.amount});
        },
        .computeEffect = [this](PassContext& ctx, const Render::DrawList&, const EffectStep& step,
                                RenderGraph::ResourceId source, RenderGraph::ResourceId destination,
                                RenderGraph::ResourceId stepBuffer)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Compute effect"};
            m_effectPass.Record(
                ctx.cmd,
                ctx.resources->View(source),
                destination.IsValid() ? ctx.resources->View(destination) : vk::ImageView{},
                stepBuffer.IsValid() ? ctx.resources->Buffer(stepBuffer).buffer : vk::Buffer{},
                destination.IsValid() ? ctx.resources->Extent(destination) : vk::Extent2D{},
                {.effect = step.effect, .amount = step.amount});
        },
        .resolveEffects = [this](PassContext& ctx, const Render::DrawList& view,
                                 RenderGraph::ResourceId source, RenderGraph::ResourceId destination)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Resolve effects"};
            // `destination` is the view's target: the pipeline for its format
            const GpuRenderTarget* target = m_device.Resources().Find(view.target);
            if (!target)
                return;
            m_compositor.RecordFullscreen(
                ctx.cmd,
                ctx.resources->View(source),
                ctx.resources->View(destination),
                target->target.Format(),
                ctx.resources->Extent(destination),
                {.effect = PostProcess::Effect::Passthrough, .amount = 1.0f});
        },
        .presentation = Vulkan::PresentationInputs{.enabled = true, .compositeScene = m_compositeScene},
        .composite = [this, &frame](PassContext& ctx, RenderGraph::ResourceId source,
                                    RenderGraph::ResourceId destination)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Composite"};
            // no source: RecordFullscreen only clears
            m_compositor.RecordFullscreen(
                ctx.cmd,
                source.IsValid() ? ctx.resources->View(source) : vk::ImageView{},
                ctx.resources->View(destination),
                frame.swapchainImage.format,
                ctx.resources->Extent(destination),
                {.effect = PostProcess::Effect::Passthrough, .amount = 1.0f});
        }
    };

#ifdef _EDITOR
    if (overlay)
    {
        inputs.overlayReads = m_overlayReads;
        inputs.overlay = [this, overlay](PassContext& ctx, RenderGraph::ResourceId destination)
        {
            const GpuZone zone{m_vulkanContext.GetGpuProfiler(), ToQueueIndex(ctx.queue), ctx.cmd, "Overlay"};
            const vk::RenderingAttachmentInfo color{
                .imageView = ctx.resources->View(destination),
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eLoad, // on top of the composite
                .storeOp = vk::AttachmentStoreOp::eStore
            };
            ctx.cmd.beginRendering(vk::RenderingInfo{
                .renderArea = {.offset = {0, 0}, .extent = ctx.resources->Extent(destination)},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color
            });
            m_imguiPass->Record(ctx.cmd, *overlay);
            ctx.cmd.endRendering();
        };
    }
#endif

    FrameGraphBuild build = BuildFrameGraph(m_preparedFrame, inputs);
    RenderGraph::Compiled compiled = build.graph.Compile();

    // A graph that does not compile still has to consume the acquire semaphore, signal render-finished and leave the
    // swapchain image in Present. The fallback graph does exactly that through the same path: an empty frame gives
    // just the swapchain import and a composite with no source, which clears. The frame's draw lists are dropped.
    if (!compiled.ok)
    {
        std::print("{}", RenderGraph::Dump(build.graph.Data(), compiled));
        m_preparedFrame = {};
        build = BuildFrameGraph(m_preparedFrame, inputs);
        compiled = build.graph.Compile();
        if (!compiled.ok)
            throw std::logic_error("Renderer: the fallback frame graph (clear the swapchain image) does not compile");
    }

    // The frame's two binary semaphores belong to the swapchain image, so it must be the only resource synchronised
    // from outside, waited on by exactly one submission and signalled by exactly one.
    std::size_t swapchainWaits = 0;
    std::size_t swapchainSignals = 0;
    bool onlySwapchainExternal = true;
    for (const RenderGraph::Submission& submission : compiled.submissions)
    {
        for (const RenderGraph::ResourceId resource : submission.waitExternal)
        {
            onlySwapchainExternal = onlySwapchainExternal && resource == build.swapchain;
            ++swapchainWaits;
        }
        for (const RenderGraph::ResourceId resource : submission.signalExternal)
        {
            onlySwapchainExternal = onlySwapchainExternal && resource == build.swapchain;
            ++swapchainSignals;
        }
    }
    if (!onlySwapchainExternal || swapchainWaits != 1 || swapchainSignals != 1)
    {
        std::print("{}", RenderGraph::Dump(build.graph.Data(), compiled));
        throw std::logic_error("Renderer: the frame graph must wait on and signal the swapchain image exactly once, "
            "and synchronise nothing else from outside");
    }

    m_graphExecutor.Execute(
        build.graph,
        compiled,
        GraphExecutionInfo{
            .submitter = &m_device.Submitter(),
            .external = FrameSubmitter::External{
                .acquire = [&frame](RenderGraph::ResourceId) -> const Semaphore& { return *frame.imageAvailable; },
                .finished = [&frame](RenderGraph::ResourceId) -> const Semaphore&
                {
                    return *frame.swapchainImage.renderFinished;
                }
            },
            .swapchainExtent = m_wsi.SwapchainExtent()
        },
        [this, &frame](const RenderGraph::ResourceInfo& resource) { return ResolveImport(resource, frame); });
}

PhysicalResource Renderer::ResolveImport(const RenderGraph::ResourceInfo& resource, const Frame& frame) const
{
    if (resource.externalKey == SWAPCHAIN_KEY)
    {
        return {
            .image = {
                .image = frame.swapchainImage.image,
                .range = COLOR_RANGE,
                .previousUse = SWAPCHAIN_ACQUIRE_USE
            },
            .view = frame.swapchainImage.view,
            .extent = frame.swapchainImage.extent
        };
    }

    const Render::TargetHandle handle = TargetFromKey(resource.externalKey);
    const GpuRenderTarget* target = m_device.Resources().Find(handle);
    if (!target)
        throw std::runtime_error(std::format(
            "render graph resource '{}' has no render target (handle index {}, generation {}, valid: {})",
            resource.name, handle.index, handle.generation, handle.IsValid()));

    return {
        .image = {
            .image = target->target.GetImage(),
            .range = target->target.GetImageResource().GetSubresourceRange(vk::ImageAspectFlagBits::eColor)
        },
        .view = target->target.GetImageView(),
        .extent = target->target.Extent()
    };
}

#ifdef _EDITOR
namespace
{
    /// @brief Convert imgui texture id to guid
    GUID ImageOfImGuiTextureId(std::uint64_t id)
    {
        return GUID{ id };
    }
}

void Renderer::InitImGui()
{
    if (!m_imguiPass)
        m_imguiPass.emplace(m_vulkanContext, m_wsi.SwapchainFormat(), WSI::FramesInFlight());
}

void Renderer::NewImGuiFrame()
{
    if (m_imguiPass)
        m_imguiPass->NewFrame();
}

std::vector<GUID> Renderer::ImGuiImages(const ImDrawData& overlay)
{
    std::vector<GUID> images;
    for (const ImDrawList* list : overlay.CmdLists)
    {
        for (const ImDrawCmd& command : list->CmdBuffer)
        {
            if (command.UserCallback || command.TexRef._TexData)
                continue;
            const GUID image = ImageOfImGuiTextureId(command.TexRef._TexID);
            if (image != Droplet::C_INVALID_GUID && std::ranges::find(images, image) == images.end())
                images.push_back(image);
        }
    }
    return images;
}

std::vector<Graphics::Render::TargetHandle> Renderer::ResolveImGuiTextures(ImDrawData& overlay, const Render::RenderWorld& world)
{
    GpuResourceManager& gpu = m_device.Resources();
    std::vector<Render::TargetHandle> reads;    
    for (ImDrawList* list : overlay.CmdLists)
    {
        for (ImDrawCmd& command : list->CmdBuffer)
        {
            // ImGui's own textures (its font atlas) carry their data; callbacks draw nothing
            const GUID image = ImageOfImGuiTextureId(command.TexRef._TexID);
            if (command.UserCallback || command.TexRef._TexData || image == Droplet::C_INVALID_GUID)
                continue;

            // a camera's target, else an idle target (its last image), else a texture asset
            Render::TargetHandle handle{};
            Render::TextureHandle textureHandle{};
            const auto camera = std::ranges::find(world.cameras, image,
                                                  [](const Common::CameraRenderData& data) { return data.target.id; });
            if (camera != world.cameras.end() && camera->target.extent.width != 0 && camera->target.extent.height != 0)
                handle = gpu.GetOrCreateTarget(camera->target);
            else
                handle = gpu.FindTarget(image);
            if (!handle.IsValid())
                textureHandle = gpu.FindTexture(image);

            vk::ImageView view;
            vk::Sampler sampler = gpu.DefaultSampler();
            if (const GpuRenderTarget* target = handle.IsValid() ? gpu.Find(handle) : nullptr)
            {
                view = target->target.GetImageView();
                if (std::ranges::find(reads, handle) == reads.end())
                    reads.push_back(handle);
            }
            else if (const GpuTexture* texture = textureHandle.IsValid() ? gpu.Find(textureHandle) : nullptr)
            {
                view = texture->view.Get(); // already shader-readable
                sampler = texture->sampler;
            }
            else
            {
                const GpuTexture* placeholder = gpu.Find(gpu.PlaceholderTexture());
                view = placeholder->view.Get();
                sampler = placeholder->sampler;
            }
            const vk::DescriptorSet set = m_imguiPass->SetFor(view, sampler);
            command.TexRef._TexID = static_cast<ImTextureID>(
                reinterpret_cast<std::uintptr_t>(static_cast<VkDescriptorSet>(set)));
        }
    }
    return reads;
}
#endif
