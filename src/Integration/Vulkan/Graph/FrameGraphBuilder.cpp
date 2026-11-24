#include "Integration/Vulkan/Graph/FrameGraphBuilder.hpp"

#include <algorithm>
#include <unordered_map>

namespace Vulkan
{
    using Render::DrawList;
    using Render::TargetHandle;
    using RenderGraph::PassBuilder;
    using RenderGraph::ResourceId;
    using RenderGraph::ImportOptions;
    using RenderGraph::Usage;

    std::vector<EffectStep> StepsFor(const Common::PostEffectDesc& effect)
    {
        using Effect = PostProcess::Effect;
        switch (effect.effect)
        {
        case Common::PostEffect::Grayscale: return {{Effect::Grayscale, effect.amount, EffectTechnique::Fragment}};
        case Common::PostEffect::Contrast: return {{Effect::Contrast, effect.amount, EffectTechnique::Fragment}};
        case Common::PostEffect::Invert: return {{Effect::Invert, effect.amount, EffectTechnique::Fragment}};
        case Common::PostEffect::Blur:
            return {{Effect::BlurHorizontal, effect.amount, EffectTechnique::Compute},
                    {Effect::BlurVertical, effect.amount, EffectTechnique::Compute}};
        case Common::PostEffect::AutoExposure:
            return {{Effect::MeasureLuminance, effect.amount, EffectTechnique::Compute, false, StepBufferUse::Writes},
                    {Effect::ApplyExposure, effect.amount, EffectTechnique::Compute, true, StepBufferUse::Reads}};
        }
        return {};
    }

    FrameGraphBuild BuildFrameGraph(
        const Render::PreparedFrame& frame,
        const FrameGraphInputs& inputs)
    {
        ZoneScoped
        const std::vector<DrawList>& views = frame.views;

        FrameGraphBuild out;

        // Collect names of every view target for debugging
        std::unordered_map<std::uint64_t, std::string> names;
        for (const DrawList& view : views)
        {
            names.emplace(TargetKey(view.target), view.name + " output");
            if (view.history.IsValid())
            {
                names.emplace(TargetKey(view.history), view.name + " history");
            }
        }

        // Get ids of non-transient targets
        std::unordered_map<std::uint64_t, ResourceId> ids;

        const auto importTarget = [&](TargetHandle handle) -> ResourceId
        {
            if (!handle.IsValid())
                throw std::logic_error("BuildFrameGraph: an invalid target handle reached importTarget "
                    "(Vulkan::Prepare only hands out valid ones: check DrawList::target, history and reads)");

            const std::uint64_t key = TargetKey(handle);
            if (const auto importedIt = ids.find(key); importedIt != ids.end())
                return importedIt->second;

            const auto name = names.contains(key)
                                  ? names[key]
                                  : "target#" + std::to_string(handle.index);

            // targets are drawn on the graphics queue and copied on the transfer queue
            // GpuResourceManager creates them shared between the queue families
            ImportOptions options{
                .concurrent = true
            };
            if (inputs.isDisplayed && inputs.isDisplayed(handle))
            {
                options.requiredFinal = Usage::SampledRead;
            }

            const ResourceId id = out.graph.Import(name, key,
                                                   inputs.currentState
                                                       ? inputs.currentState(handle)
                                                       : RenderGraph::ResourceState{},
                                                   options
            );

            ids.emplace(key, id);
            return id;
        };

        // iterate over every view and collect their read dependencies
        for (const DrawList& view : views)
        {
            const ResourceId self = importTarget(view.target);
            const ResourceId history = view.history.IsValid()
                                           ? importTarget(view.history)
                                           : ResourceId{};

            // track reads
            std::vector<ResourceId> reads;
            reads.reserve(view.reads.size());
            for (const auto target : view.reads)
            {
                reads.push_back(importTarget(target));
            }
            // loop-breaking reads: last frame's copy of another view's target, refreshed by that view's history pass
            std::vector<ResourceId> previousReads;
            previousReads.reserve(view.previousReads.size());
            for (const auto previousHistory : view.previousReads)
            {
                previousReads.push_back(importTarget(previousHistory));
            }
            const bool displayed = inputs.isDisplayed && inputs.isDisplayed(view.target);

            // the passes of the view's effects, in order; compute steps need a format for the images they write
            std::vector<EffectStep> effectSteps;
            for (const Common::PostEffectDesc& effect : view.effects)
            {
                for (const EffectStep& step : StepsFor(effect))
                {
                    if (step.technique == EffectTechnique::Fragment || inputs.effectFormat != 0)
                        effectSteps.push_back(step);
                }
            }

            // with effects the view draws into a transient; the effect chain ends in the view's target
            const bool hasEffects = !effectSteps.empty() && inputs.targetFormat;
            const ResourceId drawTarget = hasEffects
                                              ? out.graph.Create(view.name + " scene", {
                                                                     .sizeClass = RenderGraph::SizeClass::InputRelative,
                                                                     .relativeTo = self,
                                                                     .format = inputs.targetFormat(view.target)
                                                                 })
                                              : self;

            // if depth should be enabled we flag to create transient image resource
            const ResourceId depth = inputs.depthFormat != 0
                                         ? out.graph.Create(view.name + " depth", {
                                                                .sizeClass = RenderGraph::SizeClass::InputRelative,
                                                                .relativeTo = self,
                                                                .format = inputs.depthFormat
                                                            })
                                         : ResourceId{};

            // add render pass to the graph for this render view
            out.graph.AddPass(
                "View: " + view.name,
                [&](PassBuilder& builder)
                {
                    for (const ResourceId resourceId : reads)
                    {
                        builder.Read(resourceId, Usage::SampledRead);
                    }
                    if (history.IsValid())
                    {
                        builder.ReadPrevious(history, Usage::SampledRead);
                    }
                    for (const ResourceId resourceId : previousReads)
                    {
                        builder.ReadPrevious(resourceId, Usage::SampledRead);
                    }
                    builder.Write(drawTarget, Usage::ColorWrite);
                    if (depth.IsValid())
                    {
                        builder.Write(depth, Usage::DepthWrite);
                    }
                    if (displayed && !hasEffects)
                    {
                        builder.SideEffect();
                    }
                },
                [draw = inputs.drawView, &view, targets = ViewTargets{.color = drawTarget, .depth = depth}](
                PassContext& ctx)
                {
                    if (draw)
                    {
                        draw(ctx, view, targets);
                    }
                });

            // the view's effects: one pass per step, every output a new resource (single writer).
            //  - a fragment step runs on the graphics queue and writes an image of the target's format: the view's
            //    target if it is the last step, else a transient;
            //  - a compute step runs on the compute queue and writes a transient of the effect format, which unlike
            //    the target's sRGB format can be a storage image, and/or the effect's step buffer, a transient buffer
            //    that carries a result to a later step of the same effect (auto exposure's measured luminance).
            ResourceId effectSource = drawTarget;
            ResourceId stepBuffer{};
            for (std::size_t stepIndex = 0; hasEffects && stepIndex < effectSteps.size(); ++stepIndex)
            {
                const EffectStep& step = effectSteps[stepIndex];
                const bool lastStep = stepIndex + 1 == effectSteps.size();
                const bool computeStep = step.technique == EffectTechnique::Compute;
                const std::string stepName = PostProcess::ToString(step.effect);
                if (lastStep && !step.writesImage)
                    throw std::logic_error("BuildFrameGraph: an effect chain must end in a step that writes an image");
                if (step.buffer != StepBufferUse::None && !computeStep)
                    throw std::logic_error("BuildFrameGraph: only compute steps can use a step buffer");

                const ResourceId destination = !step.writesImage
                                                   ? ResourceId{}
                                                   : lastStep && !computeStep
                                                   ? self
                                                   : out.graph.Create(view.name + " " + stepName, {
                                                                          .sizeClass =
                                                                          RenderGraph::SizeClass::InputRelative,
                                                                          .relativeTo = self,
                                                                          .format = computeStep
                                                                              ? inputs.effectFormat
                                                                              : inputs.targetFormat(view.target)
                                                                      });
                if (step.buffer == StepBufferUse::Writes)
                    stepBuffer = out.graph.CreateBuffer(view.name + " " + stepName + " buffer",
                                                        {.size = STEP_BUFFER_SIZE});
                if (step.buffer == StepBufferUse::Reads && !stepBuffer.IsValid())
                    throw std::logic_error("BuildFrameGraph: effect step '" + stepName + "' reads a step buffer "
                        "that no earlier step writes");
                const ResourceId usedBuffer = step.buffer != StepBufferUse::None ? stepBuffer : ResourceId{};

                out.graph.AddPass(
                    "Effect: " + view.name + " " + stepName,
                    [&](PassBuilder& builder)
                    {
                        builder.Read(effectSource, Usage::SampledRead);
                        if (computeStep)
                        {
                            builder.OnQueue(RenderGraph::Queue::Compute);
                            if (destination.IsValid())
                                builder.Write(destination, Usage::StorageWrite);
                            if (step.buffer == StepBufferUse::Writes)
                                builder.Write(usedBuffer, Usage::StorageWrite);
                            else if (step.buffer == StepBufferUse::Reads)
                                builder.Read(usedBuffer, Usage::StorageRead);
                        }
                        else
                        {
                            builder.Write(destination, Usage::ColorWrite);
                            if (lastStep && displayed)
                            {
                                builder.SideEffect();
                            }
                        }
                    },
                    [fragmentEffect = inputs.fragmentEffect, computeEffect = inputs.computeEffect, computeStep, &view,
                        step,
                        source = effectSource, destination, usedBuffer](PassContext& ctx)
                    {
                        if (computeStep && computeEffect)
                            computeEffect(ctx, view, step, source, destination, usedBuffer);
                        else if (!computeStep && fragmentEffect)
                            fragmentEffect(ctx, view, step, source, destination);
                    });

                // a step that only writes its buffer leaves the image for the next step as it was
                if (destination.IsValid())
                    effectSource = destination;
            }

            // a chain that ends on the compute queue: a graphics pass writes its last image into the view's target
            if (hasEffects && effectSteps.back().technique == EffectTechnique::Compute)
            {
                out.graph.AddPass(
                    "Resolve: " + view.name,
                    [&](PassBuilder& builder)
                    {
                        builder.Read(effectSource, Usage::SampledRead);
                        builder.Write(self, Usage::ColorWrite);
                        if (displayed)
                        {
                            builder.SideEffect();
                        }
                    },
                    [resolveEffects = inputs.resolveEffects, &view, source = effectSource, destination = self](
                    PassContext& ctx)
                    {
                        if (resolveEffects)
                        {
                            resolveEffects(ctx, view, source, destination);
                        }
                    });
            }

            // refresh the history for next frame, on the transfer queue; kept alive by ReadPrevious above and
            // ordered after it
            if (history.IsValid())
            {
                out.graph.AddPass("History: " + view.name,
                                  [&](PassBuilder& builder)
                                  {
                                      builder.OnQueue(RenderGraph::Queue::Transfer);
                                      builder.Read(self, Usage::TransferSrc);
                                      builder.Write(history, Usage::TransferDst);
                                  },
                                  [copy = inputs.copyTarget, src = view.target, dst = view.history](
                                  PassContext& ctx)
                                  {
                                      if (copy)
                                      {
                                          copy(ctx, src, dst);
                                      }
                                  });
            }
        }

        // presentation: the composite into the swapchain image
        if (inputs.presentation.enabled)
        {
            // A new image every frame whose old contents are not needed: imported as unused (Usage::None). The
            // acquire semaphore is waited on before the first use, render-finished is signalled after the last, and
            // the graph's epilogue leaves the image in Present.
            out.swapchain = out.graph.Import(
                "Swapchain",
                SWAPCHAIN_KEY,
                RenderGraph::ResourceState{},
                ImportOptions{
                    .requiredFinal = Usage::Present,
                    .waitExternal = true,
                    .signalExternal = true
                });

            // what the composite samples: the presented view's target, which already has the view's effects
            const auto presentedView = std::ranges::find(views, true, &DrawList::presented);
            const ResourceId source = inputs.presentation.compositeScene && presentedView != views.end() 
            ? importTarget(presentedView->target) 
            : ResourceId{};

            // writes a resource with a required final usage, so the graph keeps it (and everything it reads) alive
            const ResourceId swapchain = out.swapchain;
            out.graph.AddPass(
                "Composite",
                [&](PassBuilder& builder)
                {
                    if (source.IsValid())
                    {
                        builder.Read(source, Usage::SampledRead);
                    }
                    builder.Write(swapchain, Usage::ColorWrite);
                },
                [composite = inputs.composite, source, swapchain](PassContext& ctx)
                {
                    if (composite)
                    {
                        composite(ctx, source, swapchain);
                    }
                });

            // the overlay (the editor's ImGui) on top: it reads the render targets its windows show, so the views
            // that render them come first and stay alive
            if (inputs.overlay)
            {
                std::vector<ResourceId> overlayReads;
                for (const TargetHandle read : inputs.overlayReads)
                    overlayReads.push_back(importTarget(read));
                out.graph.AddPass(
                    "Overlay",
                    [&](PassBuilder& builder)
                    {
                        for (const ResourceId read : overlayReads)
                            builder.Read(read, Usage::SampledRead);
                        builder.Write(swapchain, Usage::ColorWrite);
                    },
                    [overlay = inputs.overlay, swapchain](PassContext& ctx)
                    {
                        overlay(ctx, swapchain);
                    });
            }
        }

        return out;
    }
}
