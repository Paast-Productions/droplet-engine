#include "IntermediaryStorage.hpp"

#include <vector>

#include <ImGui/imgui_impl_vulkan.h>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyVulkan.hpp>

#include <Graphics/VK/ShaderModule.hpp> 

using namespace Droplet::Graphics::VK;

void IntermediaryStorage::Initialise(const Context &p_context)
{
    m_context = &p_context;
    m_allocator = { m_context->GetInstance(), m_context->GetPhysicalDevice(), m_context->GetDevice() };
    m_descriptorPool = { m_context->GetDevice() };
    m_textureSampler = { m_context->GetPhysicalDevice(), m_context->GetDevice() };
    
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eUniformBuffer);
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eStorageBuffer);
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eCombinedImageSampler);
}

void IntermediaryStorage::Shutdown()
{
    // Reverse deinit (Order matters)
    for (const vk::DescriptorSet &p_descriptorSet : m_imguiTextures)
    {
        ImGui_ImplVulkan_RemoveTexture(p_descriptorSet);
    }
    m_imguiTextures.clear();
    
    m_textureSampler = nullptr;
    
    // STORAGE
    m_renderTargets.clear();
    m_viewMap.clear();
    m_imageMap.clear();
    m_worldMap.clear();
    m_indexMap.clear();
    m_vertexMap.clear();
    m_pipelineMap.clear();
    
    // VULKAN
    m_descriptorSets.clear();
    m_textureSampler = { nullptr };
    m_descriptorPool = { nullptr };
    m_allocator = { nullptr };
    m_context = { nullptr };
}

std::uint32_t IntermediaryStorage::CreateRenderTarget(std::uint32_t p_width, std::uint32_t p_height)
{
    m_renderTargets.emplace_back(
        m_allocator.Get(),
        m_context->GetDevice(),
        m_context->GetPhysicalDevice(),
        p_width,
        p_height);
    
    return static_cast<std::uint32_t>(m_renderTargets.size() - 1);
}

std::uint32_t IntermediaryStorage::CreateImGuiTexture(const RenderTarget &p_renderTarget)
{
    m_imguiTextures.emplace_back(
        ImGui_ImplVulkan_AddTexture(
            *m_textureSampler.Get(),
            *p_renderTarget.GetColorImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        )
    );
    
    return static_cast<std::uint32_t>(m_imguiTextures.size() - 1);
}

void IntermediaryStorage::UpdateImGuiRenderTarget(std::uint32_t p_targetIndex, uint32_t p_textureIndex, std::uint32_t p_width, std::uint32_t p_height)
{
    m_context->GetDevice().waitIdle();
    
    ImGui_ImplVulkan_RemoveTexture(m_imguiTextures.at(p_textureIndex));
    
    m_renderTargets.emplace(m_renderTargets.begin() + p_targetIndex,
        m_allocator.Get(),
        m_context->GetDevice(),
        m_context->GetPhysicalDevice(),
        p_width,
        p_height);
    
    m_imguiTextures.emplace(m_imguiTextures.begin() + p_textureIndex,
        ImGui_ImplVulkan_AddTexture(
            *m_textureSampler.Get(),
            *m_renderTargets.at(p_targetIndex).GetColorImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        )
    );
}

const Pipeline &IntermediaryStorage::GetPipeline(RenderData p_renderData, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat)
{
    ZoneScoped;
    // TRY FINDING ALREADY CREATED PIPELINE
    
    PipelineKey key 
    {
        .vsGuid = p_renderData.vsData.GUID,
        .tcsGuid = p_renderData.tcsData.GUID,
        .tesGuid = p_renderData.tesData.GUID,
        .gsGuid = p_renderData.gsData.GUID,
        .fsGuid = p_renderData.fsData.GUID
    };
    
    auto it = m_pipelineMap.find(key);

    if (it != m_pipelineMap.end())
    {
        return m_pipelineMap.at(key);
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
        return nullptr;
    }

    std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> shaderVec{};

    // Vertex Shader

    ShaderModule vsModule{ nullptr };
    if (vs.IsReady())
    {
        ShaderResource* vsRes = vs.Get();
        vsModule = ShaderModule{ p_device, vsRes->GetByteCode() };
        shaderVec.emplace_back(vsModule.Get(), vk::ShaderStageFlagBits::eVertex);
    }
    else if (vs.IsValid())
    {
        return nullptr;
    }

    ShaderModule tcsModule{ nullptr };
    if (tcs.IsReady())
    {
        ShaderResource* tcsRes = tcs.Get();
        tcsModule = ShaderModule{ p_device, tcsRes->GetByteCode() };
        shaderVec.emplace_back(tcsModule.Get(), vk::ShaderStageFlagBits::eTessellationControl);
    }
    else if (tcs.IsValid())
    {
        return nullptr;
    }

    ShaderModule tesModule{ nullptr };
    if (tes.IsReady())
    {
        ShaderResource* tesRes = tes.Get();
        tesModule = ShaderModule{ p_device, tesRes->GetByteCode() };
        shaderVec.emplace_back(tesModule.Get(), vk::ShaderStageFlagBits::eTessellationEvaluation);
    }
    else if (tes.IsValid())
    {
        return nullptr;
    }

    ShaderModule gsModule{ nullptr };
    if (gs.IsReady())
    {
        ShaderResource* gsRes = gs.Get();
        gsModule = ShaderModule{ p_device, gsRes->GetByteCode() };
        shaderVec.emplace_back(gsModule.Get(), vk::ShaderStageFlagBits::eGeometry);
    }
    else if (gs.IsValid())
    {
        return nullptr;
    }

    ShaderModule fsModule{ nullptr };
    if (fs.IsReady())
    {
        ShaderResource* fsRes = fs.Get();
        fsModule = ShaderModule{ p_device, fsRes->GetByteCode() };
        shaderVec.emplace_back(fsModule.Get(), vk::ShaderStageFlagBits::eFragment);
    }
    else if (fs.IsValid())
    {
        return nullptr;
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
    ZoneScoped;

    MeshKey key
    {
        .meshGUID = p_renderData.meshData.GUID
    };
    
    auto it = m_indexMap.find(key);

    if (it == m_indexMap.end())
    {
        ResourceManager& RMInstance = ResourceManager::GetInstance();
        ResourceHandle<MeshResource> meshRes = RMInstance.LoadResource<MeshResource>(p_renderData.meshData.GUID);

        if (!(meshRes.IsReady() || meshRes.HasFailed()))
        {
            Mesh m;
            m.valid = false;
            return m;
        }

        m_vertexMap[key] = VertexBuffer{ m_allocator.Get(), meshRes.Get()->GetVertexData() };
        m_indexMap[key] = IndexBuffer{ m_allocator.Get(), meshRes.Get()->GetIndexData() };

        Mesh mesh
        {
            .valid = true,
            .vertexBuffer = m_vertexMap.at(key).Get(),
            .indexBuffer = m_indexMap.at(key).Get(),
            .indexCount = m_indexMap.at(key).GetIndexCount()
        };

        return mesh;
    }
    else
    {
        Mesh mesh
        {
            .valid = true,
            .vertexBuffer = m_vertexMap.at(key).Get(),
            .indexBuffer = m_indexMap.at(key).Get(),
            .indexCount = m_indexMap.at(key).GetIndexCount()
        };
        return mesh;
    }
}

std::uint32_t IntermediaryStorage::GetTextureID(RenderData p_renderData, CommandPool &p_commandPool, bool &result)
{
    ZoneScoped;

    std::uint64_t guid = p_renderData.texData.GUID;
    
    auto it = m_textureGuidMap.find(guid);

    if (it != m_textureGuidMap.end())
    {
        result = true;
        return m_textureGuidMap.at(guid);
    }

    ResourceManager& RMInstance = ResourceManager::GetInstance();
    ResourceHandle<Texture2DResource> texRes = RMInstance.LoadResource<Texture2DResource>(p_renderData.texData.GUID);

    if (!(texRes.IsReady() || texRes.HasFailed()))
    {
        result = false;
        return 0;
    }

    m_imageMap.emplace_back(
        m_allocator.Get(),
        p_commandPool,
        *m_context,
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
        m_context->GetDevice(),
        m_imageMap.back().Get(),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageAspectFlagBits::eColor
    );

    const std::uint32_t descriptorSetIndex { m_descriptorSetIndexMap.at(vk::DescriptorType::eCombinedImageSampler) };
    m_descriptorSets.at(descriptorSetIndex).WriteDescriptor(m_context->GetDevice(), m_textureSampler.Get(), m_viewMap.back().Get(), destIndex);

    m_textureGuidMap[guid] = m_viewMap.size() - 1;

    result = true;
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
