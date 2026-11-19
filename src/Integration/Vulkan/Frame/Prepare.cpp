/// @file Prepare.cpp
/// @brief Vulkan::Prepare implementation: GUIDs to GPU handles (or fallbacks), and a draw list per view.
#include "Integration/Vulkan/Frame/Prepare.hpp"
#include "Integration/Vulkan/Resources/MeshVertex.hpp"

#include <algorithm>
#include <array>
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

    /// @brief Where a dynamic draw ended up in the frame's packed geometry
    struct PackedDynamicDraw
    {
        bool packed = false; ///< false: malformed (wrong size, or an index past its vertices), not drawn
        std::uint32_t vertexByteOffset = 0;
        bool indexed = false;
        std::uint32_t first = 0;
        std::uint32_t count = 0;
    };

    std::uint32_t VertexStride(Render::DynamicVertexLayout layout)
    {
        return layout == Render::DynamicVertexLayout::P3C4
                   ? static_cast<std::uint32_t>(sizeof(Vulkan::Vertex::P3C4))
                   : static_cast<std::uint32_t>(sizeof(Vulkan::Vertex::P3N3UV2));
    }

    /// @brief Packs every dynamic draw a view shows into `frame`: vertices grouped by layout (one vertex buffer offset
    ///        per layout), indices made relative to the start of their layout's vertices
    std::vector<PackedDynamicDraw> PackDynamicDraws(const Render::RenderWorld& world, Render::PreparedFrame& frame)
    {
        std::vector<bool> shown(world.dynamicDraws.size(), false);
        for (const Render::RenderView& view : world.views)
        {
            for (const std::uint32_t drawIndex : view.dynamicVisible)
            {
                if (drawIndex < shown.size())
                    shown[drawIndex] = true;
            }
        }

        std::vector<PackedDynamicDraw> packed(world.dynamicDraws.size());
        constexpr std::array layouts{ Render::DynamicVertexLayout::P3N3UV2, Render::DynamicVertexLayout::P3C4 };
        for (const Render::DynamicVertexLayout layout : layouts)
        {
            const std::uint32_t stride = VertexStride(layout);
            frame.dynamicVertexData.resize((frame.dynamicVertexData.size() + 15) / 16 * 16);
            const auto layoutStart = static_cast<std::uint32_t>(frame.dynamicVertexData.size());
            std::uint32_t layoutVertices = 0;

            for (std::size_t drawIndex = 0; drawIndex < world.dynamicDraws.size(); ++drawIndex)
            {
                const Render::DynamicDraw& draw = world.dynamicDraws[drawIndex];
                if (draw.layout != layout || !shown[drawIndex] || draw.vertexData.empty() ||
                    draw.vertexData.size() % stride != 0)
                    continue;
                const auto drawVertices = static_cast<std::uint32_t>(draw.vertexData.size() / stride);
                if (std::ranges::any_of(draw.indices, [drawVertices](std::uint32_t index) { return index >= drawVertices; }))
                    continue;

                frame.dynamicVertexData.insert(frame.dynamicVertexData.end(), draw.vertexData.begin(), draw.vertexData.end());
                PackedDynamicDraw& result = packed[drawIndex];
                result.packed = true;
                result.vertexByteOffset = layoutStart;
                if (draw.indices.empty())
                {
                    result.first = layoutVertices;
                    result.count = drawVertices;
                }
                else
                {
                    result.indexed = true;
                    result.first = static_cast<std::uint32_t>(frame.dynamicIndices.size());
                    result.count = static_cast<std::uint32_t>(draw.indices.size());
                    for (const std::uint32_t index : draw.indices)
                        frame.dynamicIndices.push_back(layoutVertices + index);
                }
                layoutVertices += drawVertices;
            }
        }
        return packed;
    }

    /// @brief Orders a view's dynamic items (opaque, blended, no depth test) and merges neighbours that share all state
    ///        and follow each other in the packed geometry: one draw call each
    void BatchDynamicItems(std::vector<std::pair<std::uint32_t, Render::DynamicDrawItem>>& rankedItems,
                           std::vector<Render::DynamicDrawItem>& items)
    {
        std::ranges::stable_sort(rankedItems, {}, &std::pair<std::uint32_t, Render::DynamicDrawItem>::first);
        for (auto& [rank, item] : rankedItems)
        {
            if (!items.empty())
            {
                Render::DynamicDrawItem& previous = items.back();
                if (previous.pipeline == item.pipeline && previous.material == item.material &&
                    previous.model == item.model && previous.vertexByteOffset == item.vertexByteOffset &&
                    previous.indexed == item.indexed && previous.first + previous.count == item.first)
                {
                    previous.count += item.count;
                    continue;
                }
            }
            items.push_back(item);
        }
    }

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
    frame.time = world.time;

    // 2b. dynamic geometry, packed once for every view; the vertex colour shader only when something uses it
    const std::vector<PackedDynamicDraw> packedDynamic = PackDynamicDraws(world, frame);
    const bool usesVertexColor = std::ranges::any_of(world.dynamicDraws, [](const Render::DynamicDraw& draw)
    {
        return draw.layout == Render::DynamicVertexLayout::P3C4;
    });
    const bool vertexColorReady = usesVertexColor &&
        context.shaders.GetOrCreate(DrawPass::VERTEX_COLOR_SHADER, DrawPass::VertexColorShaderAsset());

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
        const glm::mat4 cameraToWorld = glm::inverse(view.view);

        Render::DrawList list{
            .name = view.name,
            .viewProj = view.proj * view.view,
            .clearColor = view.clearColor,
            .cameraPosition = glm::vec3(cameraToWorld[3]),
            .cameraForward = -glm::normalize(glm::vec3(cameraToWorld[2])), // the camera looks down its -Z
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
        std::vector<std::pair<float, Render::DrawItem>> transparentItems; // with their view-space depth
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

            const Render::RenderObject& source = world.objects[objectIndex];
            Render::DrawItem item{
                .model = source.transform,
                .pipeline = context.pipelines.GetOrCreate(context.scenePass.MakePipelineDesc(
                    object.shader, colorFormat, context.depthFormat, source.transparent)),
                .mesh = object.mesh,
                .material = material
            };
            if (source.transparent)
                transparentItems.emplace_back((view.view * source.transform[3]).z, std::move(item));
            else
                list.items.push_back(std::move(item));
        }
        SortForBinding(list.items);

        // dynamic geometry the view shows
        std::vector<std::pair<std::uint32_t, Render::DynamicDrawItem>> rankedDynamic;
        for (const std::uint32_t drawIndex : view.dynamicVisible)
        {
            if (drawIndex >= packedDynamic.size() || !packedDynamic[drawIndex].packed)
                continue;
            const Render::DynamicDraw& draw = world.dynamicDraws[drawIndex];
            const PackedDynamicDraw& packed = packedDynamic[drawIndex];

            const bool vertexColor = draw.layout == Render::DynamicVertexLayout::P3C4;
            if (vertexColor && !vertexColorReady)
                continue;
            const Guid shader = vertexColor ? DrawPass::VERTEX_COLOR_SHADER : context.shaders.Resolve(draw.shaderId, defaultShader);
            if (!shader.IsValid())
                continue;
            // render targets need the view's read edges, which dynamic draws do not declare yet
            const Render::ImageBinding material = vertexColor
                ? Render::ImageBinding{ gpu.WhiteTexture() }
                : draw.imageTexture.kind == Common::ImageRef::Kind::RenderTarget
                      ? Render::ImageBinding{ gpu.PlaceholderTexture() }
                      : ResolveImage(draw.imageTexture, gpu, targetHandles);

            const bool blended = vertexColor || draw.transparent;
            const std::uint32_t rank = !draw.depthTest ? 2u : (blended ? 1u : 0u);
            rankedDynamic.emplace_back(rank, Render::DynamicDrawItem{
                .model = draw.transform,
                .pipeline = context.pipelines.GetOrCreate(context.scenePass.MakeDynamicPipelineDesc(
                    shader, draw.layout, draw.topology, colorFormat, context.depthFormat, draw.transparent, draw.depthTest)),
                .material = material,
                .vertexByteOffset = packed.vertexByteOffset,
                .indexed = packed.indexed,
                .first = packed.first,
                .count = packed.count
            });
        }
        BatchDynamicItems(rankedDynamic, list.dynamicItems);

        // transparent draws blend over what is behind them: farthest first (view space looks down -Z)
        std::ranges::stable_sort(transparentItems, {}, &std::pair<float, Render::DrawItem>::first);
        for (auto& [depth, item] : transparentItems)
            list.items.push_back(std::move(item));

        frame.views.push_back(std::move(list));
    }

    return frame;

    return frame;
}
