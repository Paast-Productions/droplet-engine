#include "ShaderStorage.hpp"

#include <vector>

#include <Graphics/VK/ShaderModule.hpp> 
#include <Debug/Logger.hpp>

using namespace Droplet::Graphics::VK;

ShaderStorage::ShaderStorage(VK::Context &p_context, VK::DescriptorSet &p_descriptorSet) :
    m_context(p_context),
    m_descriptorSet(p_descriptorSet),
    m_allocator(m_context.GetInstance(), m_context.GetPhysicalDevice(), m_context.GetDevice()) {}

const Pipeline &ShaderStorage::GetPipeline(RenderData p_renderData, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat)
{
    // TRY FINDING ALREADY CREATED PIPELINE
    
    PipelineKey key 
    {
        .vsGuid = p_renderData.vsData.GUID,
        .tcsGuid = p_renderData.tcsData.GUID,
        .tesGuid = p_renderData.tesData.GUID,
        .gsGuid = p_renderData.gsData.GUID,
        .fsGuid = p_renderData.fsData.GUID
    };
    
    try
    {
        return m_pipelineMap.at(key);
    } 
    catch (const std::exception &err) 
    {
        Debug::Logger& logger = Debug::Logger::GetInstance();
        logger.Log(Droplet::Debug::LogType::Info, err.what());
    }
    
    // IF FAILED, CREATE NEW
    ResourceManager& RMInstance = ResourceManager::GetInstance();
    ResourceHandle<ShaderResource> vs = RMInstance.LoadResource<ShaderResource>(key.vsGuid);
    ResourceHandle<ShaderResource> tcs = RMInstance.LoadResource<ShaderResource>(key.tcsGuid);
    ResourceHandle<ShaderResource> tes = RMInstance.LoadResource<ShaderResource>(key.tesGuid);
    ResourceHandle<ShaderResource> gs = RMInstance.LoadResource<ShaderResource>(key.gsGuid);
    ResourceHandle<ShaderResource> fs = RMInstance.LoadResource<ShaderResource>(key.fsGuid);
    
    if 
    (
        !(vs.IsReady() || vs.HasFailed())
    )
    {
        throw std::runtime_error("Shaders for pipeline not ready");
    }
    
    std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> vector {};
    
    // Vertex Shader
    
    ShaderModule vsModule { nullptr };
    if (vs.IsReady())
    {
        ShaderResource* vsRes = vs.Get();
        vsModule = ShaderModule { p_device, vsRes->GetByteCode() };
        vector.emplace_back(vsModule.Get(), vk::ShaderStageFlagBits::eVertex);
    }
    
    ShaderModule tcsModule { nullptr };
    if (tcs.IsReady())
    {
        ShaderResource* tcsRes = tcs.Get();
        tcsModule = ShaderModule { p_device, tcsRes->GetByteCode() };
        vector.emplace_back(tcsModule.Get(), vk::ShaderStageFlagBits::eTessellationControl);
    }
    
    ShaderModule tesModule { nullptr };
    if (tes.IsReady())
    {
        ShaderResource* tesRes = tes.Get();
        tesModule = ShaderModule { p_device, tesRes->GetByteCode() };
        vector.emplace_back(tesModule.Get(), vk::ShaderStageFlagBits::eTessellationEvaluation);
    }
    
    ShaderModule gsModule { nullptr };
    if (gs.IsReady())
    {
        ShaderResource* gsRes = gs.Get();
        gsModule = ShaderModule { p_device, gsRes->GetByteCode() };
        vector.emplace_back(gsModule.Get(), vk::ShaderStageFlagBits::eGeometry);
    }
    
    ShaderModule fsModule { nullptr };
    if (fs.IsReady())
    {
        ShaderResource* fsRes = fs.Get();
        fsModule = ShaderModule { p_device, fsRes->GetByteCode() };
        vector.emplace_back(fsModule.Get(), vk::ShaderStageFlagBits::eFragment);
    }
    
    std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts
    {
        {
            {
                m_descriptorSet.GetLayout()  
            }
        }
    };
    
    m_pipelineMap[key] = Pipeline 
    {
        p_device, 
        p_physicalDevice,
        vector,
        {
            .PipelineLayoutInfo 
            {
                .setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size()),
                .pSetLayouts = descriptorSetLayouts.data()
                
            },
            .SwapchainSurfaceFormat = p_surfaceFormat,
        }
    };
    return m_pipelineMap.at(key);
}