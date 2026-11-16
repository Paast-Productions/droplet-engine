/**
* @file PipelineLibrary.hpp
 * @brief GUID-keyed shader storage and hash-keyed pipeline storage.
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Common/RenderDesc.hpp"
#include "Assets/AssetTransfer.hpp"
#include "Core/Guid.hpp"
#include "Core/Handle.hpp"
#include "Vulkan/Resources/GraphicsPipeline.hpp"
#include "Vulkan/Abstraction/Shader.hpp"
#include "Render/Handles.hpp"

class ShaderCompiler;

namespace Vulkan
{
    /// Owns every Shader, keyed by asset GUID and stage; compiles shader descriptions (Common::ShaderDesc) on demand.
    /// @details Resolve hands out a compiled shader or the default one. A shader is compiled once: a newer revision
    ///          of a compiled shader is reported and ignored (no hot reload yet); a newer revision of one that failed
    ///          is compiled again (when it is uploaded).
    class ShaderLibrary
    {
    public:
        explicit ShaderLibrary(const vk::raii::Device& device);
        ~ShaderLibrary();

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        /// @brief Compiles the asset's vertex and fragment stages under `id` the first time it is seen.
        /// @param revision the asset storage's revision; a failed shader is compiled again for a newer one
        /// @return false if it does not compile (reported once; later calls return false without compiling again)
        bool GetOrCreate(const Guid& id, const Common::ShaderDesc& asset, std::uint64_t revision = 0);

        /// @brief The default scene shader: the asset storage's override under `overrideId` once it has arrived, else
        ///        `builtIn` (compiled under `builtInId` on first use).
        void SetDefault(const Guid& overrideId, const Guid& builtInId, Common::ShaderDesc builtIn);

        /// @brief The default shader to draw with this frame: the override (the asset storage's shader under the
        ///        default's GUID, which the uploader sends) once it is compiled, else the built-in one. Invalid if not
        ///        even the built-in one compiles.
        [[nodiscard]] Guid ResolveDefault();

        /// @brief `id` if it is compiled, else `fallback` (also for an invalid `id`).
        [[nodiscard]] Guid Resolve(const Guid& id, const Guid& fallback);

        /// @brief Adds the compiled shaders and the failed ones, with their revisions, to `resident`.
        void AppendResident(Assets::ResidentAssets& resident) const;

        /// Adds or replaces (hot reload) the shader for this GUID and stage.
        const Shader& Set(const Guid& id, const ShaderCreateInfo& info);

        /// @brief True if both stages of `id` are compiled.
        [[nodiscard]] bool IsReady(const Guid& id) const;
        /// @brief True if `id` was given to GetOrCreate and did not compile (retried only for a newer revision).
        [[nodiscard]] bool HasFailed(const Guid& id) const { return m_failed.contains(id); }

        [[nodiscard]] const Shader* Find(const Guid& id, vk::ShaderStageFlagBits stage) const;
        [[nodiscard]] const Shader& Get(const Guid& id, vk::ShaderStageFlagBits stage) const; ///< throws if missing

        /// The Slang compiler, for shaders outside the library (the compositor's).
        [[nodiscard]] const ShaderCompiler& Compiler() const { return *m_compiler; }

    private:
        struct StageKeyHash
        {
            std::size_t operator()(const std::pair<Guid, vk::ShaderStageFlagBits>& key) const noexcept
            {
                return std::hash<Guid>{}(key.first) ^ (static_cast<std::size_t>(key.second) << 1);
            }
        };

        const vk::raii::Device& m_device;
        std::unique_ptr<ShaderCompiler> m_compiler;
        std::unordered_map<std::pair<Guid, vk::ShaderStageFlagBits>, std::unique_ptr<Shader>, StageKeyHash> m_shaders;
        std::unordered_map<Guid, std::uint64_t> m_failed;    ///< assets that did not compile, at this revision
        std::unordered_map<Guid, std::uint64_t> m_revisions; ///< revision each compiled asset was compiled from
        std::unordered_map<Guid, std::uint64_t> m_ignored;   ///< newest ignored revision per asset (reported once)

        Guid m_defaultOverride;           ///< the asset storage's default shader (Render::DEFAULT_SCENE_SHADER)
        Guid m_builtInDefault;            ///< the renderer's own default shader
        Common::ShaderDesc m_builtInAsset;
    };

    /**
     * @brief Builds pipelines on demand and caches them.
     *
     * Key = desc.Hash() (shader GUIDs, layout, vertex layout, formats, fixed state), checked against the full
     * description on a hit. Pipelines are requested during Prepare and bound by handle when drawing.
     */
    class PipelineLibrary
    {
    public:
        PipelineLibrary(const vk::raii::Device& device, const ShaderLibrary& shaders);

        /// @brief The pipeline for `desc`, built the first time. @throws std::runtime_error if a shader is missing
        Render::PipelineHandle GetOrCreate(const GraphicsPipelineDesc& desc);

        [[nodiscard]] const GraphicsPipeline* Find(Render::PipelineHandle handle) const;

        /// Drops every pipeline. Caller must make sure the GPU no longer uses them.
        void Clear();

    private:
        struct Entry
        {
            GraphicsPipelineDesc desc;
            GraphicsPipeline pipeline;
        };

        const vk::raii::Device& m_device;
        const ShaderLibrary& m_shaders;
        vk::raii::PipelineCache m_vkCache{nullptr}; ///< Driver-side cache; can be persisted to disk later
        HandlePool<Render::PipelineTag, Entry> m_pipelines;
        std::unordered_map<std::uint64_t, std::vector<Render::PipelineHandle>> m_lookup; ///< by desc.Hash()
    };
}
