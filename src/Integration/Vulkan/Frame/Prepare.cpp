/// @file Prepare.cpp
/// @brief Vulkan::Prepare implementation: GUIDs to GPU handles (or fallbacks), and a draw list per view.
#include "Integration/Vulkan/Frame/Prepare.hpp"

#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace
{
    /// @brief Target handle for `targetId`, or an invalid handle if the world has no description for it.
    Render::TargetHandle ResolveTarget(
        Guid targetId,
        const std::unordered_map<Guid, Render::TargetHandle>& targetHandles)
    {
        const auto targetIt = targetHandles.find(targetId);
        return targetIt != targetHandles.end() ? targetIt->second : Render::TargetHandle{};
    }

    /// @brief The image an object shows. What is missing becomes the placeholder texture; no image at
    ///        all is the white texture.
    Render::ImageBinding ResolveImage(
        const Common::ImageRef& image,
        Vulkan::GpuResourceManager& gpu,
        const std::unordered_map<Guid, Render::TargetHandle>& targetHandles)
    {
        switch (image.kind)
        {
        case Common::ImageRef::Kind::Texture:
            return gpu.ResolveTexture(image.id);
        case Common::ImageRef::Kind::RenderTarget:
            if (const Render::TargetHandle target = ResolveTarget(image.id, targetHandles); target.IsValid())
                return target;
            return gpu.PlaceholderTexture(); // a target the scene does not declare
        case Common::ImageRef::Kind::None:
            break;
        }
        return gpu.WhiteTexture(); // no texture on purpose: not a missing one
    }

    /// @brief A RenderObject's GUIDs as what is drawn for them. Mesh and base colour are always valid.
    struct ResolvedObject
    {
        Render::MeshHandle mesh;
        Render::ImageBinding baseColor;
        Guid shader; ///< a compiled shader; invalid: not even the default compiled
    };

    /// @brief A target that a view samples as its history (last frame's copy) instead.
    struct HistorySubstitute
    {
        Render::TargetHandle target;
        Render::TargetHandle history; ///< invalid: none could be created
    };

    /// @brief Orders `items` as Render::DrawList::items documents; equal items keep their order.
    void SortForBinding(std::vector<Render::DrawItem>& items)
    {
        const auto sortKey = [](const Render::DrawItem& item)
        {
            // textures, then render targets, then unresolved (an invalid texture handle)
            const Render::TextureHandle* texture = std::get_if<Render::TextureHandle>(&item.material);
            const Render::TargetHandle* target = std::get_if<Render::TargetHandle>(&item.material);
            const std::uint32_t rank = texture ? (texture->IsValid() ? 0u : 2u) : 1u;
            const std::uint32_t material = rank == 0 ? texture->index : (rank == 1 ? target->index : 0u);
            return std::tuple{item.pipeline.index, rank, material, item.mesh.index};
        };
        std::ranges::stable_sort(items,
                                 [&](const Render::DrawItem& lhs, const Render::DrawItem& rhs)
                                 {
                                     return sortKey(lhs) < sortKey(rhs);
                                 });
    }
}

Render::PreparedFrame Vulkan::Prepare(
    const Render::RenderWorld& world,
    const Vulkan::PrepareContext& context)
{
    GpuResourceManager& gpu = context.gpu;

    // the default shader: the asset storage's override once it has arrived, else the built-in one
    const Guid defaultShader = context.shaders.ResolveDefault();

    // 1. render targets, from the descriptions the world carries
    std::unordered_map<Guid, Render::TargetHandle> targetHandles;
    for (const Common::RenderTargetDesc& desc : world.targets)
    {
        targetHandles.emplace(desc.id, gpu.GetOrCreateTarget(desc));
    }

    // 2. objects: mesh, base colour and shader; what is missing is drawn as a placeholder. Every object is resolved,
    //    seen by a view or not
    std::vector<ResolvedObject> objects;
    objects.reserve(world.objects.size());
    for (const Render::RenderObject& object : world.objects)
    {
        objects.push_back({
            .mesh = gpu.ResolveMesh(object.meshId),
            .baseColor = ResolveImage(object.imageTexture, gpu, targetHandles),
            // a shader that has not arrived or does not compile is replaced by the default
            .shader = context.shaders.Resolve(object.shaderId, defaultShader)
        });
    }

    Render::PreparedFrame frame;

    // 3. views: one draw list each
    const Render::TargetHandle presented = ResolveTarget(world.presentedId, targetHandles);
    bool presentedFound = false;

    frame.views.reserve(world.views.size());
    for (const Render::RenderView& view : world.views)
    {
        const Render::TargetHandle target = ResolveTarget(view.targetId, targetHandles);
        const GpuRenderTarget* gpuTarget = gpu.Find(target);
        if (!gpuTarget)
            continue; // nothing to draw into
        // read now: creating a history below can grow the target pool and move gpuTarget
        const vk::Format colorFormat = gpuTarget->target.Format();

        Render::DrawList list{
            .name = view.name,
            .viewProj = view.proj * view.view,
            .target = target,
            .history = view.needsHistory ? gpu.GetOrCreateHistory(target) : Render::TargetHandle{},
            .presented = !presentedFound && target == presented,
            .effects = view.effects,
        };
        presentedFound = presentedFound || list.presented;
        
        // other views' targets, this frame's contents
        for (const Guid readId : view.readIds)
        {
            if (const Render::TargetHandle read = ResolveTarget(readId, targetHandles); read.IsValid())
                list.reads.push_back(read);
        }

        // targets sampled as last frame's copy: the view's own (a view cannot sample what it draws into), and reads
        // that would close a loop between views (the same history the other view refreshes)
        std::vector<HistorySubstitute> substitutes;

        substitutes.push_back({.target = target, .history = list.history});

        for (const Guid previousId : view.previousReadIds)
        {
            const Render::TargetHandle previous = ResolveTarget(previousId, targetHandles);
            if (!previous.IsValid())
                continue;
            const Render::TargetHandle history = gpu.GetOrCreateHistory(previous);

            substitutes.push_back({.target = previous, .history = history});

            if (history.IsValid())
                list.previousReads.push_back(history);
        }

        // draws: per visible object, with the pipeline for its shader and this view's attachment formats (built on
        // first use)
        list.items.reserve(view.visible.size());
        for (const std::uint32_t objectIndex : view.visible)
        {
            const ResolvedObject& object = objects[objectIndex];
            if (!object.shader.IsValid())
                continue; // no pipeline to draw it with

            Render::ImageBinding material = object.baseColor;
            if (const Render::TargetHandle* sampledTarget = std::get_if<Render::TargetHandle>(&material))
            {
                const auto substitute = std::ranges::find(substitutes, *sampledTarget, &HistorySubstitute::target);
                if (substitute != substitutes.end())
                {
                    material = substitute->history.IsValid()
                                   ? Render::ImageBinding{substitute->history}
                                   : Render::ImageBinding{gpu.PlaceholderTexture()};
                }
            }

            list.items.push_back({
                .model = world.objects[objectIndex].transform,
                .pipeline = context.pipelines.GetOrCreate(
                    context.scenePass.MakePipelineDesc(object.shader, colorFormat, context.depthFormat)),
                .mesh = object.mesh,
                .material = material
            });
        }
        SortForBinding(list.items);

        frame.views.push_back(std::move(list));
    }

    return frame;

    return frame;
}
