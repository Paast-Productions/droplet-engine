/**
 * @file PipelineLibrary.cpp
 * @brief Shader and pipeline caches.
 */
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"

#include <algorithm>
#include <exception>
#include <print>
#include <stdexcept>
#include <utility>

#include "Integration/ShaderCompiler/ShaderCompiler.hpp"

namespace Vulkan
{
    ShaderLibrary::ShaderLibrary(const vk::raii::Device& device)
        : m_device(device),
          m_compiler(std::make_unique<ShaderCompiler>())
    {
    }

    ShaderLibrary::~ShaderLibrary() = default;

    bool ShaderLibrary::IsReady(Droplet::GUID id) const
    {
        return Find(id, vk::ShaderStageFlagBits::eVertex) && Find(id, vk::ShaderStageFlagBits::eFragment);
    }

    bool ShaderLibrary::GetOrCreate(Droplet::GUID id, const Common::ShaderDesc& asset, std::uint64_t revision)
    {
        if (IsReady(id))
        {
            const std::uint64_t compiled = m_revisions[id];
            if (revision > compiled && m_ignored[id] < revision)
            {
                std::println("ShaderLibrary: '{}' has a newer revision ({} > {}); not reloaded (no hot reload yet)",
                             asset.moduleName, revision, compiled);
                m_ignored[id] = revision;
            }
            return true;
        }
        if (const auto failedIt = m_failed.find(id); failedIt != m_failed.end() && failedIt->second >= revision)
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
            m_failed.erase(id);
            m_revisions[id] = revision;
            return true;
        }
        catch (const std::exception& exception)
        {
            // a broken material shader must not stop the renderer: report it once, the caller falls back
            std::println("ShaderLibrary: '{}' ({}) does not compile: {}",
                         asset.moduleName, asset.sourcePath.string(), exception.what());
            m_shaders.erase({id, vk::ShaderStageFlagBits::eVertex});
            m_shaders.erase({id, vk::ShaderStageFlagBits::eFragment});
            m_failed[id] = revision;
            return false;
        }
    }

    void ShaderLibrary::SetDefault(Droplet::GUID overrideId, Droplet::GUID builtInId, Common::ShaderDesc builtIn)
    {
        m_defaultOverride = overrideId;
        m_builtInDefault = builtInId;
        m_builtInAsset = std::move(builtIn);
    }

    Droplet::GUID ShaderLibrary::ResolveDefault()
    {
        const Droplet::GUID builtIn = m_builtInDefault;
        if (!IsReady(builtIn) && !HasFailed(builtIn))
            GetOrCreate(builtIn, m_builtInAsset);
        const Droplet::GUID fallback = IsReady(builtIn) ? builtIn : Droplet::C_INVALID_GUID;
        return IsReady(m_defaultOverride) ? m_defaultOverride : fallback;
    }

    Droplet::GUID ShaderLibrary::Resolve(Droplet::GUID id, Droplet::GUID fallback)
    {
        return IsReady(id) ? id : fallback;
    }

    void ShaderLibrary::AppendResident(Assets::ResidentAssets& resident) const
    {
        for (const auto& [id, revision] : m_revisions)
        {
            if (IsReady(id))
                resident.shaders.push_back({ .id = id, .revision = revision });
        }
        for (const auto& [id, revision] : m_failed)
            resident.failedShaders.push_back({ .id = id, .revision = revision });
    }

    const Shader& ShaderLibrary::Set(Droplet::GUID id, const ShaderCreateInfo& info)
    {
        // Safe even if pipelines were built from the previous shader: modules are not
        // referenced by pipelines after creation.
        auto& slot = m_shaders[{id, info.stage}];
        slot = std::make_unique<Shader>(m_device, info);
        return *slot;
    }

    const Shader* ShaderLibrary::Find(Droplet::GUID id, vk::ShaderStageFlagBits stage) const
    {
        const auto shaderIt = m_shaders.find({id, stage});
        return shaderIt != m_shaders.end() ? shaderIt->second.get() : nullptr;
    }

    const Shader& ShaderLibrary::Get(Droplet::GUID id, vk::ShaderStageFlagBits stage) const
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

        const Shader& vertex = m_shaders.Get(desc.vertexShader, vk::ShaderStageFlagBits::eVertex);
        const Shader* fragment = desc.hasFragmentShader
                                     ? &m_shaders.Get(desc.fragmentShader, vk::ShaderStageFlagBits::eFragment)
                                     : nullptr;

        const Render::PipelineHandle handle = m_pipelines.Add(Entry{
            .desc = desc,
            .pipeline = GraphicsPipeline(m_device, desc, vertex, fragment, &m_vkCache)});
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
