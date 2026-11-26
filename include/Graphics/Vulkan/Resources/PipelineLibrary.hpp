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

#include "Core/Guid.hpp"
#include "Core/Handle.hpp"
#include "ResourceBridge/ResourceTransfer.hpp"
#include "Common/RenderDesc.hpp"
#include "Graphics/Vulkan/Resources/GraphicsPipeline.hpp"
#include "Graphics/Vulkan/Abstraction/Shader.hpp"
#include "Graphics/Render/Handles.hpp"

namespace Droplet::IO
{
    class ShaderCompiler;
}

namespace Droplet::Graphics::Vulkan
{
    /**
     * @brief Owns every Shader, keyed by GUID and stage
     * @details Uploaded shaders arrive as one compiled stage each; the renderer's built-in shaders are compiled from
     *          source, both stages under one GUID. A shader is created once: a newer revision of a created one is
     *          reported and ignored (no hot reload yet); a newer revision of one that failed is tried again
     */
    class ShaderLibrary
    {
    public:
        explicit ShaderLibrary(const vk::raii::Device& device, bool geometryShaders, bool tessellationShaders);
        ~ShaderLibrary();

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        /**
         * @brief Creates an uploaded stage the first time it is seen
         * @param upload One compiled stage
         * @returns false if it is not valid SPIR-V (reported once per revision)
         */
        bool Upload(const Bridge::ShaderUpload& upload);

        /**
         * @brief Compiles a built-in shader's vertex and fragment stages under `id` the first time it is seen
         * @param id The built-in shader's GUID
         * @param asset The Slang module and its entry points
         * @returns false if it does not compile (reported once)
         */
        bool GetOrCreate(GUID id, const Common::ShaderDesc& asset);

        /// @brief The renderer's own default, compiled on first use: drawn with when not even production's default is ready
        void SetBuiltInDefault(GUID builtInId, Common::ShaderDesc builtIn);

        /**
         * @brief The default group to draw with this frame
         * @param defaultShaders The group production names (RenderWorld)
         * @returns It once all stages are created, else the built-in default; invalid if not even that compiles
         */
        [[nodiscard]] ShaderGroup ResolveDefault(const ShaderGroup &defaultShaders);

        /// @brief `shaders` if all stages are created, else `fallback`
        [[nodiscard]] ShaderGroup Resolve(const ShaderGroup &shaders, const ShaderGroup &fallback) const;

        /// @brief Adds the created shaders and the failed ones, with their revisions, to `resident`.
        void AppendResident(Bridge::ResidentResources& resident) const;

        /// Adds or replaces (hot reload) the shader for this GUID and stage.
        const Shader& Set(GUID id, const ShaderCreateInfo& info);

        /// @brief True if `id` has `stage` created
        [[nodiscard]] bool IsReady(GUID id, vk::ShaderStageFlagBits stage) const { return Find(id, stage) != nullptr; }
        /// @brief True if `id` was tried and failed (tried again only for a newer revision)
        [[nodiscard]] bool HasFailed(GUID id) const { return m_failed.contains(id); }
        [[nodiscard]] const Shader* Find(GUID id, vk::ShaderStageFlagBits stage) const;
        [[nodiscard]] const Shader& Get(GUID id, vk::ShaderStageFlagBits stage) const; ///< throws if missing

        /// The Slang compiler, for shaders outside the library (the compositor's).
        [[nodiscard]] const Droplet::IO::ShaderCompiler& Compiler() const { return *m_compiler; }

    private:
        struct StageKeyHash
        {
            std::size_t operator()(const std::pair<GUID, vk::ShaderStageFlagBits>& key) const noexcept
            {
                return std::hash<GUID>{}(key.first) ^ (static_cast<std::size_t>(key.second) << 1);
            }
        };

        const vk::raii::Device& m_device;
        std::unique_ptr<IO::ShaderCompiler> m_compiler;
        bool m_geometryShaders; ///< the device has the geometry stage
        bool m_tessellationShaders; ///< the device has the tessellation stages
        std::unordered_map<std::pair<GUID, vk::ShaderStageFlagBits>, std::unique_ptr<Shader>, StageKeyHash> m_shaders; ///< hashes shader groups to stage bits
        std::unordered_map<GUID, std::uint64_t> m_failed; ///< assets that did not compile, at this revision
        std::unordered_map<GUID, std::uint64_t> m_revisions; ///< revision each compiled asset was compiled from
        std::unordered_map<GUID, std::uint64_t> m_ignored; ///< newest ignored revision per asset (reported once)

        GUID m_builtInDefault{ C_INVALID_GUID }; ///< the renderer's own default shader
        Common::ShaderDesc m_builtInResource;
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
