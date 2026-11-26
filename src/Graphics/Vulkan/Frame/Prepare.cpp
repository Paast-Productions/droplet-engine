/// @file Prepare.cpp
/// @brief Vulkan::Prepare implementation: GUIDs to GPU handles (or fallbacks), and a draw list per view.
#include "Graphics/Vulkan/Frame/Prepare.hpp"
#include "Graphics/Vulkan/Resources/MeshVertex.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <print>
#include <span>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>


using namespace Droplet;
using namespace Droplet::Graphics;

namespace
{
    /// @brief Target handle for `targetId`, or an invalid handle if the world has no description for it.
    Render::TargetHandle ResolveTarget(
        GUID targetId,
        const std::unordered_map<GUID, Render::TargetHandle>& targetHandles)
    {
        const auto targetIt = targetHandles.find(targetId);
        return targetIt != targetHandles.end() ? targetIt->second : Render::TargetHandle{};
    }

    /// @brief The image an object shows: missing is the placeholder, none at all is white
    Render::ImageBinding ResolveImage(
        const Common::TextureData& texture,
        Vulkan::GpuResourceManager& gpu,
        const std::unordered_map<GUID, Render::TargetHandle>& targetHandles)
    {
        if (texture.GUID == C_INVALID_GUID)
            return gpu.WhiteTexture(); // no texture on purpose: not a missing one
        if (!texture.isRenderTarget)
            return gpu.ResolveTexture(texture.GUID);
        if (const Render::TargetHandle target = ResolveTarget(texture.GUID, targetHandles); target.IsValid())
            return target;
        if (const Render::TargetHandle idle = gpu.FindTarget(texture.GUID); idle.IsValid())
            return idle; // its last image
        return gpu.PlaceholderTexture(); // never rendered
    }

    /**
     * @brief Whether an object uses a tessellation or geometry stage, which the renderer does not support
     * @param object The object to check
     * @returns The first such stage's GUID, or C_INVALID_GUID
     */
    GUID UnsupportedStageOf(const Common::RenderData& object)
    {
        for (const GUID stage : { object.tcsData.guid, object.tesData.guid, object.gsData.guid })
            if (stage != C_INVALID_GUID)
                return stage;
        return C_INVALID_GUID;
    }

    /// @brief What is drawn for an object's GUIDs; mesh and base colour are always valid
    struct ResolvedObject
    {
        Render::MeshHandle mesh;
        Render::ImageBinding baseColor;
        Vulkan::ShaderGroup shaders; ///< compiled stages; invalid: not even the default compiled
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

    /**
     * @brief Packs every streamed dynamic mesh a drawn camera shows into `frame`, once
     * @details Vertices are grouped by stride (one vertex buffer offset per stride); indices are made relative to the
     *          start of their group's vertices. Malformed meshes are left out
     * @returns Where each packed mesh ended up
     */
    std::unordered_map<const Common::DynamicMeshData*, PackedDynamicDraw> PackStreamedMeshes(
        const Render::RenderWorld& world, std::span<const Render::ViewPlan> plans, Render::PreparedFrame& frame)
    {
        std::vector<const Common::DynamicMeshData*> meshes;
        std::vector<std::uint32_t> strides;
        for (std::size_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
        {
            if (!plans[cameraIndex].drawn)
                continue;
            for (const Common::RenderData& object : world.cameras[cameraIndex].renderData)
            {
                const Common::DynamicMeshData* mesh = object.dynamicMesh.get();
                if (!mesh || mesh->id != Droplet::C_INVALID_GUID || std::ranges::find(meshes, mesh) != meshes.end())
                    continue;
                meshes.push_back(mesh);
                if (std::ranges::find(strides, mesh->vertexStride) == strides.end())
                    strides.push_back(mesh->vertexStride);
            }
        }

        std::unordered_map<const Common::DynamicMeshData*, PackedDynamicDraw> packed;
        for (const std::uint32_t stride : strides)
        {
            frame.dynamicVertexData.resize((frame.dynamicVertexData.size() + 15) / 16 * 16);
            const auto groupStart = static_cast<std::uint32_t>(frame.dynamicVertexData.size());
            std::uint32_t groupVertices = 0;

            for (const Common::DynamicMeshData* mesh : meshes)
            {
                if (mesh->vertexStride != stride || stride == 0 || mesh->vertexData.empty() ||
                    mesh->vertexData.size() % stride != 0)
                    continue;
                const auto meshVertices = static_cast<std::uint32_t>(mesh->vertexData.size() / stride);
                if (std::ranges::any_of(mesh->indices, [meshVertices](std::uint32_t index) { return index >= meshVertices; }))
                    continue;

                frame.dynamicVertexData.insert(frame.dynamicVertexData.end(), mesh->vertexData.begin(), mesh->vertexData.end());
                PackedDynamicDraw& result = packed[mesh];
                result.packed = true;
                result.vertexByteOffset = groupStart;
                if (mesh->indices.empty())
                {
                    result.first = groupVertices;
                    result.count = meshVertices;
                }
                else
                {
                    result.indexed = true;
                    result.first = static_cast<std::uint32_t>(frame.dynamicIndices.size());
                    result.count = static_cast<std::uint32_t>(mesh->indices.size());
                    for (const std::uint32_t index : mesh->indices)
                        frame.dynamicIndices.push_back(groupVertices + index);
                }
                groupVertices += meshVertices;
            }
        }
        return packed;
    }

    /// @brief A key for reporting a dynamic mesh once: its id, or its layout if it is streamed
    std::uint64_t ReportKey(const Common::DynamicMeshData& mesh)
    {
        if (mesh.id != Droplet::C_INVALID_GUID)
            return mesh.id;
        std::uint64_t key = mesh.vertexStride;
        for (const Common::VertexAttribute& attribute : mesh.attributes)
            key = key * 1099511628211ull ^ (attribute.location << 16 | static_cast<std::uint32_t>(attribute.format) << 8 | attribute.offset);
        return key;
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
    std::span<const Render::ViewPlan> plans,
    const Vulkan::PrepareContext& context)
{
    ZoneScoped
    
    GpuResourceManager& gpu = context.gpu;

    // the default shaders: the world's once they have arrived, else the built-in ones
    const ShaderGroup defaultShaders = context.shaders.ResolveDefault(ShaderGroup(world.defaultVertexShader, world.defaultFragmentShader));

    // 1. render targets: one per drawn camera
    std::unordered_map<GUID, Render::TargetHandle> targetHandles;
    for (std::size_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
    {
        if (plans[cameraIndex].drawn)
        {
            const Droplet::Common::RenderTargetDesc& desc = world.cameras[cameraIndex].target;
            targetHandles.emplace(desc.id, gpu.GetOrCreateTarget(desc));
        }
    }

    Render::PreparedFrame frame;
    frame.time = world.time;

    // 2. streamed dynamic meshes, packed once
    const auto packedStreamed = PackStreamedMeshes(world, plans, frame);
    const bool usesVertexColor = std::ranges::any_of(world.cameras, [](const Common::CameraRenderData& camera)
    {
        return std::ranges::any_of(camera.renderData, [](const Common::RenderData& object)
        {
            return object.dynamicMesh && DrawPass::InterfaceOf(*object.dynamicMesh) == DrawPass::MeshInterface::VertexColor;
        });
    });
    const bool vertexColorReady = usesVertexColor &&
        context.shaders.GetOrCreate(DrawPass::VERTEX_COLOR_SHADER, DrawPass::VertexColorShaderAsset());

    // 3. views: one draw list per drawn camera
    const Render::TargetHandle presented = ResolveTarget(world.presentedId, targetHandles);
    bool presentedFound = false;

    frame.views.reserve(world.cameras.size());
    for (std::size_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
    {
        const Common::CameraRenderData& camera = world.cameras[cameraIndex];
        const Render::ViewPlan& plan = plans[cameraIndex];
        if (!plan.drawn)
            continue;
        const Render::TargetHandle target = ResolveTarget(camera.target.id, targetHandles);
        const GpuRenderTarget* gpuTarget = gpu.Find(target);
        if (!gpuTarget)
            continue; // nothing to draw into
        // read now: creating a history below can grow the target pool and move gpuTarget
        const vk::Format colorFormat = gpuTarget->target.Format();
        const glm::mat4 view = camera.camera.GetViewMatrix();
        const glm::mat4 proj = camera.camera.GetProjectionMatrix();

        const auto settings = std::ranges::find(world.viewSettings, camera.target.id, &Render::ViewSettings::targetId);
        Render::DrawList list{
            .name = settings != world.viewSettings.end() ? settings->name : std::format("Camera {:016x}", camera.target.id),
            .view = view,
            .proj = proj,
            .clearColor = camera.clearColor,
            .cameraPosition = camera.camera.GetPosition(),
            .cameraForward = glm::normalize(camera.camera.GetForward()),
            .target = target,
            .history = plan.needsHistory ? gpu.GetOrCreateHistory(target) : Render::TargetHandle{},
            .presented = !presentedFound && target == presented,
            .effects = settings != world.viewSettings.end() ? settings->effects : std::vector<Common::PostEffectDesc>{},
        };
        presentedFound = presentedFound || list.presented;
        
        // other views' targets, this frame's contents
        for (const GUID readId : plan.readIds)
        {
            if (const Render::TargetHandle read = ResolveTarget(readId, targetHandles); read.IsValid())
                list.reads.push_back(read);
        }

        // targets sampled as last frame's copy: the view's own (a view cannot sample what it draws into), and reads
        // that would close a loop between views (the same history the other view refreshes)
        std::vector<HistorySubstitute> substitutes;

        substitutes.push_back({.target = target, .history = list.history});

        for (const GUID previousId : plan.previousReadIds)
        {
            const Render::TargetHandle previous = ResolveTarget(previousId, targetHandles);
            if (!previous.IsValid())
                continue;
            const Render::TargetHandle history = gpu.GetOrCreateHistory(previous);

            substitutes.push_back({.target = previous, .history = history});

            if (history.IsValid())
                list.previousReads.push_back(history);
        }

        // an object's texture: a target the view draws into, or reads across a loop, is sampled as its history
        const auto materialOf = [&](const Common::TextureData& texture)
        {
            Render::ImageBinding material = ResolveImage(texture, gpu, targetHandles);
            if (const Render::TargetHandle* sampledTarget = std::get_if<Render::TargetHandle>(&material))
            {
                const auto substitute = std::ranges::find(substitutes, *sampledTarget, &HistorySubstitute::target);
                if (substitute != substitutes.end())
                {
                    material = substitute->history.IsValid()
                                   ? Render::ImageBinding{substitute->history}
                                   : Render::ImageBinding{gpu.PlaceholderTexture()};
                }
                else if (std::ranges::find(list.reads, *sampledTarget) == list.reads.end())
                {
                    list.reads.push_back(*sampledTarget); // idle target
                }
            }
            return material;
        };

        // draws: per object the camera sees, with the pipeline for its shader and this view's attachment formats
        // (built on first use)
        list.items.reserve(camera.renderData.size());
        std::vector<std::pair<float, Render::DrawItem>> transparentItems; // with their view-space depth
        std::vector<std::pair<std::uint32_t, Render::DynamicDrawItem>> rankedDynamic;
        for (const Common::RenderData& source : camera.renderData)
        {
            if (const GUID unsupported = UnsupportedStageOf(source); unsupported != C_INVALID_GUID)
            {
                if (context.reportedMeshes.insert(unsupported).second)
                    std::println("Prepare: shader {:016x} is a tessellation or geometry stage, which is not supported. "
                                 "Objects using it are not drawn", unsupported);
                continue;
            }
            const ShaderGroup objectShaders{ source.vsData.guid, source.fsData.guid, source.gsData.guid, source.tcsData.guid, source.tesData.guid };

            if (source.dynamicMesh)
            {
                const Common::DynamicMeshData& mesh = *source.dynamicMesh;
                const DrawPass::MeshInterface meshInterface = DrawPass::InterfaceOf(mesh);
                if (meshInterface == DrawPass::MeshInterface::None)
                {
                    if (context.reportedMeshes.insert(ReportKey(mesh)).second)
                        std::println("Prepare: dynamic mesh {:016x} has a vertex layout no built-in shader reads (position "
                                     "float3 at 0, then normal float3 and uv float2, or a float4 or unorm8x4 colour). Not drawn",
                                     mesh.id);
                    continue;
                }
                const bool vertexColor = meshInterface == DrawPass::MeshInterface::VertexColor;
                if (vertexColor && !vertexColorReady)
                    continue;
                const ShaderGroup shaders = vertexColor
                    ? ShaderGroup(DrawPass::VERTEX_COLOR_SHADER, DrawPass::VERTEX_COLOR_SHADER)
                    : context.shaders.Resolve(objectShaders, defaultShaders);
                if (!shaders.IsValid())
                    continue;

                const Render::ImageBinding material = vertexColor ? Render::ImageBinding{ gpu.WhiteTexture() } : materialOf(source.texData);
                const Render::PipelineHandle pipeline = context.pipelines.GetOrCreate(context.scenePass.MakeMeshPipelineDesc(
					shaders.vertex.id, shaders.fragment.id, shaders.tessCtrl.id, shaders.tessEval.id, shaders.geometry.id,
                    mesh, colorFormat, context.depthFormat, source.transparent));
                const bool blended = vertexColor || source.transparent;

                if (mesh.id != C_INVALID_GUID)
                {
                    // kept on the GPU: uploaded before Prepare
                    const Render::MeshHandle handle = gpu.FindDynamicMesh(mesh.id);
                    if (!handle.IsValid())
                        continue;
                    Render::DrawItem item{ .model = source.transform, .pipeline = pipeline, .mesh = handle, .material = material };
                    if (!mesh.depthTest)
                        list.lateItems.push_back(std::move(item));
                    else if (blended)
                        transparentItems.emplace_back((view * source.transform[3]).z, std::move(item));
                    else
                        list.items.push_back(std::move(item));
                    continue;
                }

                // streamed: packed for this frame
                const auto packed = packedStreamed.find(&mesh);
                if (packed == packedStreamed.end())
                    continue;
                const std::uint32_t rank = !mesh.depthTest ? 2u : (blended ? 1u : 0u);
                rankedDynamic.emplace_back(rank, Render::DynamicDrawItem{
                    .model = source.transform,
                    .pipeline = pipeline,
                    .material = material,
                    .vertexByteOffset = packed->second.vertexByteOffset,
                    .indexed = packed->second.indexed,
                    .first = packed->second.first,
                    .count = packed->second.count
                });
                continue;
            }

            // no mesh on purpose (e.g. a particle emitter): nothing to draw. A mesh that is missing shows the placeholder
            if (source.meshData.GUID == Droplet::C_INVALID_GUID)
            {
                continue;
            }

            const ResolvedObject object{
                .mesh = gpu.ResolveMesh(source.meshData.GUID),
                .baseColor = materialOf(source.texData),
                // a shader that has not arrived or does not compile is replaced by the default
                .shaders = context.shaders.Resolve(objectShaders, defaultShaders)
            };
            if (!object.shaders.IsValid())
                continue; // no pipeline to draw it with

            const Render::ImageBinding material = object.baseColor;
            Render::DrawItem item{
                .model = source.transform,
                .pipeline = context.pipelines.GetOrCreate(context.scenePass.MakePipelineDesc(
                    object.shaders.vertex.id, object.shaders.fragment.id, object.shaders.tessCtrl.id, object.shaders.tessEval.id, object.shaders.geometry.id, 
                    colorFormat, context.depthFormat, source.transparent)),
                .mesh = object.mesh,
                .material = material
            };
            if (source.transparent)
                transparentItems.emplace_back((view * source.transform[3]).z, std::move(item));
            else
                list.items.push_back(std::move(item));
        }
        SortForBinding(list.items);

        BatchDynamicItems(rankedDynamic, list.dynamicItems);

        // transparent draws blend over what is behind them: farthest first (view space looks down -Z)
        std::ranges::stable_sort(transparentItems, {}, &std::pair<float, Render::DrawItem>::first);
        for (auto& [depth, item] : transparentItems)
            list.items.push_back(std::move(item));

        frame.views.push_back(std::move(list));
    }

    return frame;
}
