#include "IntermediaryStorage.hpp"

#include <vector>

#include <Graphics/VK/ShaderModule.hpp> 
#include <Debug/Logger.hpp>

using namespace Droplet::Graphics::VK;

IntermediaryStorage::IntermediaryStorage(Context &p_context) :
    m_context(p_context),
    m_allocator(m_context.GetInstance(), m_context.GetPhysicalDevice(), m_context.GetDevice()),
    m_descriptorPool(m_context.GetDevice()),
    m_textureSampler(m_context.GetPhysicalDevice(), m_context.GetDevice())
{
    // TODO: <REFACTOR>
    // Should be a for-loop
    m_descriptorSets.emplace_back(m_context.GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eUniformBuffer);
    m_descriptorSets.emplace_back(m_context.GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eStorageBuffer);
    m_descriptorSets.emplace_back(m_context.GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eCombinedImageSampler);
    // TODO: </REFACTOR>
}

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
    catch (const std::exception &) 
    {
        /*Debug::Logger& logger = Debug::Logger::GetInstance();
        logger.Log(Droplet::Debug::LogType::Info, err.what());*/
    }
    
    // IF FAILED, CREATE NEW
    ResourceManager &RMInstance = ResourceManager::GetInstance();
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
    
    std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> shaderVec {};
    
    // Vertex Shader
    
    ShaderModule vsModule { nullptr };
    if (vs.IsReady())
    {
        ShaderResource* vsRes = vs.Get();
        vsModule = ShaderModule { p_device, vsRes->GetByteCode() };
        shaderVec.emplace_back(vsModule.Get(), vk::ShaderStageFlagBits::eVertex);
    }
    else if (vs.IsValid())
    {
        throw std::runtime_error("Vertex Shader is not ready");
    }
    
    ShaderModule tcsModule { nullptr };
    if (tcs.IsReady())
    {
        ShaderResource* tcsRes = tcs.Get();
        tcsModule = ShaderModule { p_device, tcsRes->GetByteCode() };
        shaderVec.emplace_back(tcsModule.Get(), vk::ShaderStageFlagBits::eTessellationControl);
    }
    else if (tcs.IsValid())
    {
        throw std::runtime_error("Tessellation Control Shader is not ready");
    }
    
    ShaderModule tesModule { nullptr };
    if (tes.IsReady())
    {
        ShaderResource* tesRes = tes.Get();
        tesModule = ShaderModule { p_device, tesRes->GetByteCode() };
        shaderVec.emplace_back(tesModule.Get(), vk::ShaderStageFlagBits::eTessellationEvaluation);
    }
    else if (tes.IsValid())
    {
        throw std::runtime_error("Tessellation Evaluation Shader is not ready");
    }
    
    ShaderModule gsModule { nullptr };
    if (gs.IsReady())
    {
        ShaderResource* gsRes = gs.Get();
        gsModule = ShaderModule { p_device, gsRes->GetByteCode() };
        shaderVec.emplace_back(gsModule.Get(), vk::ShaderStageFlagBits::eGeometry);
    }
    else if (gs.IsValid())
    {
        throw std::runtime_error("Geometry Shader is not ready");
    }
    
    ShaderModule fsModule { nullptr };
    if (fs.IsReady())
    {
        ShaderResource* fsRes = fs.Get();
        fsModule = ShaderModule { p_device, fsRes->GetByteCode() };
        shaderVec.emplace_back(fsModule.Get(), vk::ShaderStageFlagBits::eFragment);
    } 
    else if (fs.IsValid())
    {
        throw std::runtime_error("Fragment Shader is not ready");
    }
    
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
    for (DescriptorSet& descriptorSet : m_descriptorSets)
    {
        descriptorSetLayouts.push_back(*descriptorSet.GetLayout());
    }
    
    m_pipelineMap[key] = Pipeline 
    {
        p_device, 
        p_physicalDevice,
        descriptorSetLayouts,
        shaderVec,
        p_surfaceFormat
    };
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

std::uint32_t IntermediaryStorage::GetTextureID(RenderData p_renderData, CommandPool &p_commandPool)
{
    std::uint64_t guid = p_renderData.texData.GUID;
    
    try
    {
        return m_textureGuidMap.at(guid);
    }
    catch (const std::exception& err)
    {
        Debug::Logger& logger = Debug::Logger::GetInstance();
        logger.Log(Droplet::Debug::LogType::Info, err.what());
    }

    ResourceManager& RMInstance = ResourceManager::GetInstance();
    ResourceHandle<Texture2DResource> texRes = RMInstance.LoadResource<Texture2DResource>(p_renderData.texData.GUID);

    if (!(texRes.IsReady() || texRes.HasFailed()))
    {
        throw std::runtime_error("Texture not ready");
    }

    m_imageMap.emplace_back(
        m_allocator.Get(),
        p_commandPool,
        m_context,
        texRes.Get()->GetPixelData().data(),
        texRes.Get()->GetPixelData().size(),
        texRes.Get()->GetWidth(),
        texRes.Get()->GetHeight(),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    
    std::uint32_t destIndex = static_cast<std::uint32_t>(m_viewMap.size());
    
    m_viewMap.emplace_back(
        m_context.GetDevice(),
        m_imageMap.back().Get(),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageAspectFlagBits::eColor
    );
    
    try
    {
        const std::uint32_t descriptorSetIndex { m_descriptorSetIndexMap.at(vk::DescriptorType::eCombinedImageSampler) };
        m_descriptorSets.at(descriptorSetIndex).WriteDescriptor(m_context.GetDevice(), m_textureSampler.Get(), m_viewMap.back().Get(), destIndex);
    }
    catch (const std::exception &)
    {
        m_viewMap.erase(m_viewMap.end());
        m_imageMap.erase(m_imageMap.end());

        throw std::runtime_error("Failed to write image sampler descriptor!");
    }

    m_textureGuidMap[guid] = destIndex;

    return m_textureGuidMap.at(guid);
}

std::vector<vk::DescriptorSet> IntermediaryStorage::GetDescriptorSets() const
{
    std::vector<vk::DescriptorSet> descriptorSets {};
    
    for (const DescriptorSet &descriptorSet : m_descriptorSets)
    {
        descriptorSets.push_back(descriptorSet.Get());
    }
        
    return descriptorSets; 
}
