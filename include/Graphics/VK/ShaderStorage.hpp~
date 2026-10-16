#pragma once

#include <functional>

#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/Pipeline.hpp>
#include <Graphics/VK/Allocator.hpp>
#include <Graphics/VK/DescriptorSet.hpp>
#include <Graphics/RenderData.hpp>
#include <Resource/ResourceManager.hpp>

namespace Droplet::Graphics::VK
{
    struct PipelineKey
    {
        std::uint64_t vsGuid {0};
        std::uint64_t tcsGuid {0};
        std::uint64_t tesGuid {0};
        std::uint64_t gsGuid {0};
        std::uint64_t fsGuid {0};
        
        std::strong_ordering operator<=>(const PipelineKey &) const = default; 
    };
    
    struct PipelineHasher
    {
        std::size_t operator()(const PipelineKey& key) const
        {
            std::size_t res = 17;
            res = res * 31 + std::hash<std::uint64_t>()(key.vsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.tcsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.tesGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.gsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.fsGuid);
            
            return res;
            
        }
    };
    
    class ShaderStorage
    {
    public:
        ShaderStorage() = delete;
        
        ShaderStorage(VK::Context &p_context, VK::DescriptorSet &p_descriptorSet);
        
        ShaderStorage(const ShaderStorage &p_other) = delete;
        ShaderStorage &operator=(const ShaderStorage &p_other) = delete;
        ShaderStorage(ShaderStorage &&p_other) noexcept = delete;
        ShaderStorage &operator=(ShaderStorage &&p_other) noexcept = delete;
        
        ~ShaderStorage() = default;
        
        [[nodiscard]] const vma::raii::Allocator &GetAllocator() const { return m_allocator.Get(); }
        
        [[nodiscard]] const Pipeline &GetPipeline(RenderData p_renderData, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat);
        
    private:
        const VK::Context &m_context;
        const VK::DescriptorSet &m_descriptorSet;
        
        VK::Allocator m_allocator { nullptr };
        std::unordered_map<PipelineKey, Pipeline, PipelineHasher> m_pipelineMap {};
    };
}

