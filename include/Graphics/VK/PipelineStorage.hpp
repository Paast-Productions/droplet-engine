#pragma once

#include <unordered_map>

#include <vulkan/vulkan_raii.hpp>

namespace DropletEngine
{
    // Name Subject To Change
    struct PipelineStorageKey
    {
        std::uint32_t VertexShaderUUID {0};
        std::uint32_t TessellationControlShaderUUID {0};
        std::uint32_t TessellationEvaluationShaderUUID {0}; 
        std::uint32_t GeometryShaderUUID {0};
        std::uint32_t FragmentShaderUUID {0};
    };

    class PipelineStorage
    {
    public:
        PipelineStorage() = default;
        PipelineStorage(const PipelineStorage&) = delete;
        PipelineStorage& operator=(const PipelineStorage&) = delete;
        PipelineStorage(PipelineStorage&&) = delete;
        PipelineStorage& operator=(PipelineStorage&&) = delete;
        
        void Insert(const PipelineStorageKey& p_key, const vk::raii::Pipeline& p_pipeline);
        vk::raii::Pipeline& GetPipeline(const PipelineStorageKey& p_key);
        
    private:
        std::unordered_map<PipelineStorageKey, vk::raii::Pipeline> m_pipelineHashMap {};
    };
    
}
