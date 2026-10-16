#include "IntermediaryStorage.hpp"

#include <queue>

#include <Graphics/VK/ShaderModule.hpp> 
#include <Debug/Logger.hpp>

using namespace Droplet::Graphics::VK;

IntermediaryStorage::IntermediaryStorage(VK::Context &p_context) :
    m_context(p_context),
    m_allocator(m_context.GetInstance(), m_context.GetPhysicalDevice(), m_context.GetDevice()) {}

const Pipeline &IntermediaryStorage::GetPipeline(RenderData p_renderData, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat)
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
        !(vs.IsReady() || vs.HasFailed())   ||
        !(tcs.IsReady() || tcs.HasFailed()) ||
        !(tes.IsReady() || tes.HasFailed()) ||
        !(gs.IsReady() || gs.HasFailed())   ||
        !(fs.IsReady() || fs.HasFailed())
    )
    {
        throw std::runtime_error("Shaders for pipeline not ready");
    }
    
    std::queue<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> queue {};
    
    // Vertex Shader
    
    if (vs.IsReady())
    {
        ShaderResource* vsRes = vs.Get();
        ShaderModule vsModule = ShaderModule { p_device, vsRes->GetByteCode() };
        queue.emplace(vsModule.Get(), vk::ShaderStageFlagBits::eVertex);
    }
    
    if (tcs.IsReady())
    {
        ShaderResource* tcsRes = tcs.Get();
        ShaderModule tcsModule = ShaderModule { p_device, tcsRes->GetByteCode() };
        queue.emplace(tcsModule.Get(), vk::ShaderStageFlagBits::eTessellationControl);
    }
    
    if (tes.IsReady())
    {
        ShaderResource* tesRes = tes.Get();
        ShaderModule tesModule = ShaderModule { p_device, tesRes->GetByteCode() };
        queue.emplace(tesModule.Get(), vk::ShaderStageFlagBits::eTessellationEvaluation);
    }
    
    if (gs.IsReady())
    {
        ShaderResource* gsRes = gs.Get();
        ShaderModule gsModule = ShaderModule { p_device, gsRes->GetByteCode() };
        queue.emplace(gsModule.Get(), vk::ShaderStageFlagBits::eGeometry);
    }
    
    if (fs.IsReady())
    {
        ShaderResource* fsRes = fs.Get();
        ShaderModule fsModule = ShaderModule { p_device, fsRes->GetByteCode() };
        queue.emplace(fsModule.Get(), vk::ShaderStageFlagBits::eFragment);
    }
    
    m_pipelineMap[key] = Pipeline { p_device, p_physicalDevice, queue,{.SwapchainSurfaceFormat = p_surfaceFormat} };
    return m_pipelineMap.at(key);
}

const Mesh &IntermediaryStorage::GetMesh(RenderData p_renderData)
{
    MeshKey key
    {
        .meshGUID = p_renderData.meshData.GUID
    };

    try
    {
        Mesh mesh
        {
            .vertexBuffer = m_vertexMap.at(key).Get(),
            .indexBuffer = m_indexMap.at(key).Get(),
            .indexCount = m_indexMap.at(key).GetIndexCount()
        };
        return mesh;
    }
    catch (const std::exception &err)
    {
        Debug::Logger &logger = Debug::Logger::GetInstance();
        logger.Log(Droplet::Debug::LogType::Info, err.what());
    }

    ResourceManager &RMInstance = ResourceManager::GetInstance();
    ResourceHandle<MeshResource> meshRes = RMInstance.LoadResource<MeshResource>(p_renderData.meshData.GUID);

    if (!(meshRes.IsReady() || meshRes.HasFailed()))
    {
        throw std::runtime_error("Mesh not ready");
    }

    m_vertexMap[key] = VertexBuffer { m_allocator.Get(), meshRes.Get()->GetVertexData()};
    m_indexMap[key] = IndexBuffer { m_allocator.Get(), meshRes.Get()->GetIndexData()};

    Mesh mesh
    {
        .vertexBuffer = m_vertexMap.at(key).Get(),
        .indexBuffer = m_indexMap.at(key).Get(),
        .indexCount = m_indexMap.at(key).GetIndexCount()
    };

    return mesh;
}
