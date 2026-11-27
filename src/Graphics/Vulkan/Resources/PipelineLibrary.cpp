/**
 * @file PipelineLibrary.cpp
 * @brief Shader and pipeline caches.
 */
#include "Graphics/Vulkan/Resources/PipelineLibrary.hpp"
#include "Resource/IO/ShaderCompiler.hpp"

#include <algorithm>
#include <cstring>
#include <exception>
#include <print>
#include <stdexcept>
#include <utility>


namespace Droplet::Graphics::Vulkan
{
    ShaderLibrary::ShaderLibrary(const vk::raii::Device& device, bool geometryShaders, bool tessellationShaders)
        : m_device(device),
          m_compiler(std::make_unique<Droplet::IO::ShaderCompiler>()),
          m_geometryShaders(geometryShaders),
          m_tessellationShaders(tessellationShaders)
    {
    }

    ShaderLibrary::~ShaderLibrary() = default;

    bool ShaderLibrary::Upload(const Bridge::ShaderUpload& upload)
    {
        vk::ShaderStageFlagBits stage{};
        switch (upload.stage)
        {
        case Bridge::ShaderStage::Vertex: stage = vk::ShaderStageFlagBits::eVertex; break;
        case Bridge::ShaderStage::TessControl: stage = vk::ShaderStageFlagBits::eTessellationControl; break;
        case Bridge::ShaderStage::TessEvaluation: stage = vk::ShaderStageFlagBits::eTessellationEvaluation; break;
        case Bridge::ShaderStage::Geometry: stage = vk::ShaderStageFlagBits::eGeometry; break;
        case Bridge::ShaderStage::Fragment: stage = vk::ShaderStageFlagBits::eFragment; break;
        default:
            std::println("ShaderLibrary: shader {:016x} has an unknown stage; not uploaded", upload.id);
            return false;
        }
        // not recorded as failed: m_failed is per GUID, which would also block the shader's supported stages
        if ((stage == vk::ShaderStageFlagBits::eGeometry && !m_geometryShaders)
            || ((stage == vk::ShaderStageFlagBits::eTessellationControl || stage == vk::ShaderStageFlagBits::eTessellationEvaluation) && !m_tessellationShaders))
        {
            std::println("ShaderLibrary: shader {:016x} needs a stage the device does not support; not uploaded", upload.id);
            return false;
        }
        if (IsReady(upload.id, stage))
        {
            const std::uint64_t created = m_revisions[upload.id];
            if (upload.revision > created && m_ignored[upload.id] < upload.revision)
            {
                std::println("ShaderLibrary: shader {:016x} has a newer revision ({} > {}); not reloaded (no hot reload yet)",
                             upload.id, upload.revision, created);
                m_ignored[upload.id] = upload.revision;
            }
            return true;
        }
        if (const auto failedIt = m_failed.find(upload.id); failedIt != m_failed.end() && failedIt->second >= upload.revision)
            return false;

        try
        {
            constexpr std::uint32_t SPIRV_MAGIC = 0x07230203;
            if (upload.spirv.empty() || upload.spirv.size() % sizeof(std::uint32_t) != 0)
                throw std::invalid_argument("no SPIR-V, or not whole words");
            std::vector<std::uint32_t> words(upload.spirv.size() / sizeof(std::uint32_t));
            std::memcpy(words.data(), upload.spirv.data(), upload.spirv.size());
            if (words.front() != SPIRV_MAGIC)
                throw std::invalid_argument("not SPIR-V");
            Set(upload.id, ShaderCreateInfo{ .spirv = std::move(words), .stage = stage, .entrypoint = upload.entryPoint });
            m_failed.erase(upload.id);
            m_revisions[upload.id] = upload.revision;
            return true;
        }
        catch (const std::exception& exception)
        {
            // a broken shader must not stop the renderer: report it once, the objects using it draw with the default
            std::println("ShaderLibrary: shader {:016x} revision {} cannot be used: {}", upload.id, upload.revision, exception.what());
            m_failed[upload.id] = upload.revision;
            return false;
        }
    }

    bool ShaderLibrary::GetOrCreate(GUID id, const Common::ShaderDesc& asset)
    {
        if (IsReady(id, vk::ShaderStageFlagBits::eVertex) && IsReady(id, vk::ShaderStageFlagBits::eFragment))
            return true;
        if (m_failed.contains(id))
            return false;

        try
        {
            Set(id, ShaderCreateInfo{
                .spirv = m_compiler->CompileSpirv(asset.moduleName.c_str(), asset.sourcePath,
                                                  asset.vertexEntry.c_str()),
                .stage = vk::ShaderStageFlagBits::eVertex,
                .entrypoint = asset.vertexEntry});
            Set(id, ShaderCreateInfo{
                .spirv = m_compiler->CompileSpirv(asset.moduleName.c_str(), asset.sourcePath,
                                                  asset.fragmentEntry.c_str()),
                .stage = vk::ShaderStageFlagBits::eFragment,
                .entrypoint = asset.fragmentEntry});
            return true;
        }
        catch (const std::exception& exception)
        {
            // a broken built-in shader must not stop the renderer: report it once, the caller falls back
            std::println("ShaderLibrary: '{}' ({}) does not compile: {}",
                         asset.moduleName, asset.sourcePath.string(), exception.what());
            m_shaders.erase({id, vk::ShaderStageFlagBits::eVertex});
            m_shaders.erase({id, vk::ShaderStageFlagBits::eFragment});
            m_failed[id] = 0;
            return false;
        }
    }

    void ShaderLibrary::SetBuiltInDefault(GUID builtInId, Common::ShaderDesc builtIn)
    {
        m_builtInDefault = builtInId;
        m_builtInResource = std::move(builtIn);
    }

    ShaderGroup ShaderLibrary::ResolveDefault(const ShaderGroup &defaultShaders)
    {
        const GUID builtIn = m_builtInDefault;
        const ShaderGroup fallback = GetOrCreate(builtIn, m_builtInResource)
            ? ShaderGroup{ .vertex = builtIn, .fragment = builtIn }
            : ShaderGroup{};

        return Resolve(defaultShaders, fallback);
    }

    ShaderGroup ShaderLibrary::Resolve(const ShaderGroup &shaders, const ShaderGroup &fallback) const
    {
        // every stage the group uses must be created; an invalid group (no vertex or fragment stage, half a
        // tessellation pair) falls back
        const auto stageReady = [this](GUID id, vk::ShaderStageFlagBits stage)
        {
            return id == C_INVALID_GUID || IsReady(id, stage);
        };
        const bool ready = shaders.IsValid()
            && stageReady(shaders.vertex, vk::ShaderStageFlagBits::eVertex)
            && stageReady(shaders.tessControl, vk::ShaderStageFlagBits::eTessellationControl)
            && stageReady(shaders.tessEvaluation, vk::ShaderStageFlagBits::eTessellationEvaluation)
            && stageReady(shaders.geometry, vk::ShaderStageFlagBits::eGeometry)
            && stageReady(shaders.fragment, vk::ShaderStageFlagBits::eFragment);

        return ready ? shaders : fallback;
    }

    void ShaderLibrary::AppendResident(Bridge::ResidentResources& resident) const
    {
        for (const auto& [id, revision] : m_revisions)
            resident.shaders.push_back({ .id = id, .revision = revision });
        for (const auto& [id, revision] : m_failed)
            resident.failedShaders.push_back({ .id = id, .revision = revision });
    }

    const Shader& ShaderLibrary::Set(GUID id, const ShaderCreateInfo& info)
    {
        // Safe even if pipelines were built from the previous shader: modules are not
        // referenced by pipelines after creation.
        auto& slot = m_shaders[{id, info.stage}];
        slot = std::make_unique<Shader>(m_device, info);
        return *slot;
    }

    const Shader* ShaderLibrary::Find(GUID id, vk::ShaderStageFlagBits stage) const
    {
        const auto shaderIt = m_shaders.find({id, stage});
        return shaderIt != m_shaders.end() ? shaderIt->second.get() : nullptr;
    }

    const Shader& ShaderLibrary::Get(GUID id, vk::ShaderStageFlagBits stage) const
    {
        if (const Shader* shader = Find(id, stage))
            return *shader;
        throw std::runtime_error("ShaderLibrary: unknown shader GUID");
    }

    PipelineLibrary::PipelineLibrary(const vk::raii::Device& device, const ShaderLibrary& shaders)
        : m_device(device),
          m_shaders(shaders),
          m_vkCache(device, vk::PipelineCacheCreateInfo{})
    {
    }

    Render::PipelineHandle PipelineLibrary::GetOrCreate(const GraphicsPipelineDesc& desc)
    {
        const std::uint64_t key = desc.Hash();
        std::vector<Render::PipelineHandle>& candidates = m_lookup[key];
        for (const Render::PipelineHandle candidate : candidates)
        {
            if (const Entry* entry = m_pipelines.Get(candidate); entry && entry->desc == desc)
                return candidate;
        }

        // the created stage for each GUID the group uses; an unused stage stays null
        const auto stage = [this](GUID id, vk::ShaderStageFlagBits stageBit) -> const Shader*
        {
            return id != C_INVALID_GUID ? &m_shaders.Get(id, stageBit) : nullptr;
        };
        const GraphicsPipeline::Stages stages{
            .vertex = stage(desc.shaders.vertex, vk::ShaderStageFlagBits::eVertex),
            .tessControl = stage(desc.shaders.tessControl, vk::ShaderStageFlagBits::eTessellationControl),
            .tessEvaluation = stage(desc.shaders.tessEvaluation, vk::ShaderStageFlagBits::eTessellationEvaluation),
            .geometry = stage(desc.shaders.geometry, vk::ShaderStageFlagBits::eGeometry),
            .fragment = stage(desc.shaders.fragment, vk::ShaderStageFlagBits::eFragment),
        };

        const Render::PipelineHandle handle = m_pipelines.Add(Entry{
            .desc = desc,
            .pipeline = GraphicsPipeline(m_device, desc, stages, &m_vkCache)});
        candidates.push_back(handle);
        return handle;
    }

    const GraphicsPipeline* PipelineLibrary::Find(Render::PipelineHandle handle) const
    {
        const Entry* entry = m_pipelines.Get(handle);
        return entry ? &entry->pipeline : nullptr;
    }

    void PipelineLibrary::Clear()
    {
        for (const auto& [key, handles] : m_lookup)
            for (const Render::PipelineHandle handle : handles)
                m_pipelines.Remove(handle);
        m_lookup.clear();
    }
}
