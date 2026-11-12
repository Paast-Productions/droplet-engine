#include "IntermediaryStorage.hpp"

#include <vector>

#include <ImGui/imgui_impl_vulkan.h>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyVulkan.hpp>

#include <Graphics/VK/ShaderModule.hpp> 
#include <Graphics/VK/RenderTarget.hpp>
#include <Graphics/VK/ImGuiTexture.hpp>
#include <Resource/DynamicResourceHandle.hpp>

using namespace Droplet::Graphics::VK;

void IntermediaryStorage::Initialise(const Context &p_context, const CommandPool &p_commandPool)
{
    m_context = &p_context;
    m_commandPool = &p_commandPool;
    
    m_allocator = { m_context->GetInstance(), m_context->GetPhysicalDevice(), m_context->GetDevice() };
    m_descriptorPool = { m_context->GetDevice() };
    m_textureSampler = { m_context->GetPhysicalDevice(), m_context->GetDevice() };
    
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eUniformBuffer);
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eStorageBuffer);
    m_descriptorSets.emplace_back(m_context->GetDevice(), m_descriptorPool.Get(), vk::DescriptorType::eCombinedImageSampler);
}

void IntermediaryStorage::Shutdown()
{
    DestroyUnusedResources();
    m_liveObjects.clear();
    
    m_textureSampler = { nullptr };
    
    // STORAGE
    m_presentTargets.clear();
    m_uniformBuffers.clear();
    //m_renderTargets.clear();
    m_viewMap.clear();
    m_imageMap.clear();
    m_worldMap.clear();
    m_indexMap.clear();
    m_vertexMap.clear();
    m_pipelineMap.clear();
    
    // VULKAN
    m_descriptorSets.clear();
    m_descriptorPool = { nullptr };
    m_allocator = { nullptr };
    m_commandPool = { nullptr };
    m_context = { nullptr };
}


std::uint32_t IntermediaryStorage::GetLiveObjectRefCount(GUID p_guid) const
{
    if (p_guid == C_INVALID_GUID)
    {
        return 0;
    }
    
    return m_liveObjects.at(p_guid).refCount;
}

void IntermediaryStorage::IncrementRef(GUID p_guid)
{
    auto refCount = ++m_liveObjects.at(p_guid).refCount;
    
    if (refCount == 1)
    {
        TracyPlot("Vulkan Objects", static_cast<std::int64_t>(m_liveObjects.size()));   
    }
}

void IntermediaryStorage::DecrementRef(GUID p_guid)
{
    auto it = m_liveObjects.find(p_guid);
    if (it == m_liveObjects.end())
    {
        std::print(std::cerr, "Tried to decrement a non-existing Vulkan object at GUID: {0}\n", p_guid);
        std::flush(std::cerr);
        return;   
    }
    if (--m_liveObjects.at(p_guid).refCount == 0)
    {
        MoveToDestructionQueue(p_guid);
        return;
    }
}

void IntermediaryStorage::MoveToDestructionQueue(GUID p_guid)
{
    // Move unused object to a destroy queue. It will be popped once the 
    m_destroyQueue.push(std::move(m_liveObjects[p_guid]));
    m_liveObjects.erase(p_guid);
}

void IntermediaryStorage::DestroyUnusedResources()
{
    m_context->GetDevice().waitIdle();
    
    while (!m_destroyQueue.empty())
    {
        DynamicLiveResource resource = std::move(m_destroyQueue.front());
        
        // If we have specific removal rules for each object type, add them here
        switch (resource.type)
        {
            case DynamicResourceType::Buffer:
            case DynamicResourceType::Image:
            case DynamicResourceType::ImageView:
                break;
                
            case DynamicResourceType::ImGuiTexture:
                ImGui_ImplVulkan_RemoveTexture(dynamic_cast<ImGuiTextureResource*>(resource.resource.get())->Get());
                break;
            
            case DynamicResourceType::INVALID:
                throw std::runtime_error("[Intermediary Storage]: Failed To Destroy Object With Invalid Type");
        }
        
        m_destroyQueue.pop();
    }
    
    TracyPlot("Vulkan Objects", static_cast<std::int64_t>(m_liveObjects.size()));
}

Droplet::Graphics::RenderTarget IntermediaryStorage::CreateRenderTarget(std::uint32_t p_width, std::uint32_t p_height)
{
    // Create Colour Image
    
    DynamicLiveResource colourImage 
    {
        .type = DynamicResourceType::Image,
        .resource = std::make_unique<Image>(m_allocator.Get(),
            p_width,
            p_height,
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    GUID colourImageGuid { GuidUtils::Generate() };
    m_liveObjects[colourImageGuid] = std::move(colourImage);
    
    DynamicResourceHandle<Image> colourImageHandle { colourImageGuid };
    
    // Create Colour Image View
    
    DynamicLiveResource colourImageView 
    {
        .type = DynamicResourceType::ImageView,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
            colourImageHandle->Get())
    };
    
    GUID colourImageViewGuid { GuidUtils::Generate() };
    m_liveObjects[colourImageViewGuid] = std::move(colourImageView);
    
    DynamicResourceHandle<ImageView> colourImageViewHandle { colourImageViewGuid };
    
    // Create Depth Image
    
    DynamicLiveResource depthImage
    {
        .type = DynamicResourceType::Image,
        .resource = std::make_unique<Image>(m_allocator.Get(),
        p_width,
        p_height,
        vk::Format::eD32Sfloat, 
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    GUID depthImageGuid { GuidUtils::Generate() };
    m_liveObjects[depthImageGuid] = std::move(depthImage);
    
    DynamicResourceHandle<Image> depthImageHandle { depthImageGuid };
    
    // Create Depth Image View
    
    DynamicLiveResource depthImageView 
    {
        .type = DynamicResourceType::ImageView,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
        depthImageHandle->Get(),
        vk::Format::eD32Sfloat,
        vk::ImageAspectFlagBits::eDepth)
    };
    
    GUID depthImageViewGuid { GuidUtils::Generate() };
    m_liveObjects[depthImageViewGuid] = std::move(depthImageView);
    
    DynamicResourceHandle<ImageView> depthImageViewHandle { depthImageViewGuid };
    
    return RenderTarget 
    {
        colourImageHandle,
        colourImageViewHandle,
        depthImageHandle,
        depthImageViewHandle
    };
}

Droplet::Graphics::RenderTarget IntermediaryStorage::CreateRenderTarget(GUID p_colourImageGuid, GUID p_colourImageViewGuid, GUID p_depthImageGuid, GUID p_depthImageViewGuid, std::uint32_t p_width, std::uint32_t p_height)
{
    // Create Colour Image
    
    DynamicLiveResource colourImage 
    {
        .type = DynamicResourceType::Image,
        .resource = std::make_unique<Image>(m_allocator.Get(),
            p_width,
            p_height,
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    m_liveObjects[p_colourImageGuid] = std::move(colourImage);
    
    DynamicResourceHandle<Image> colourImageHandle { p_colourImageGuid };
    
    // Create Colour Image View
    
    DynamicLiveResource colourImageView 
    {
        .type = DynamicResourceType::ImageView,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
            colourImageHandle->Get())
    };
    
    m_liveObjects[p_colourImageViewGuid] = std::move(colourImageView);
    
    DynamicResourceHandle<ImageView> colourImageViewHandle { p_colourImageViewGuid };
    
    // Create Depth Image
    
    DynamicLiveResource depthImage
    {
        .type = DynamicResourceType::Image,
        .resource = std::make_unique<Image>(m_allocator.Get(),
        p_width,
        p_height,
        vk::Format::eD32Sfloat, 
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    m_liveObjects[p_depthImageGuid] = std::move(depthImage);
    
    DynamicResourceHandle<Image> depthImageHandle { p_depthImageGuid };
    
    // Create Depth Image View
    
    DynamicLiveResource depthImageView 
    {
        .type = DynamicResourceType::ImageView,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
        depthImageHandle->Get(),
        vk::Format::eD32Sfloat,
        vk::ImageAspectFlagBits::eDepth)
    };
    
    m_liveObjects[p_depthImageViewGuid] = std::move(depthImageView);
    
    DynamicResourceHandle<ImageView> depthImageViewHandle { p_depthImageViewGuid };
    
    return RenderTarget 
    {
        colourImageHandle,
        colourImageViewHandle,
        depthImageHandle,
        depthImageViewHandle
    };
}

Droplet::Graphics::ImGuiTexture IntermediaryStorage::CreateImGuiTexture(const RenderTarget &p_renderTarget)
{
    DynamicLiveResource imGuiTexture
    {
        .type = DynamicResourceType::ImGuiTexture,
        .resource = std::make_unique<ImGuiTextureResource>(ImGui_ImplVulkan_AddTexture(
            *m_textureSampler.Get(),
            *p_renderTarget.GetColourImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        )
    };
    
    GUID imGuiTextureGuid { GuidUtils::Generate() };
    m_liveObjects[imGuiTextureGuid] = std::move(imGuiTexture);
    
    DynamicResourceHandle<ImGuiTextureResource> imGuiTextureHandle { imGuiTextureGuid }; 
    
    return ImGuiTexture { imGuiTextureHandle };
}

Droplet::Graphics::ImGuiTexture IntermediaryStorage::CreateImGuiTexture(GUID p_guid)
{
    ZoneScoped;
    
    ResourceManager &RMInstance = ResourceManager::GetInstance();
    StaticResourceHandle<Texture2DResource> texRes = RMInstance.LoadResource<Texture2DResource>(p_guid);

    if (!(texRes.IsReady() || texRes.HasFailed()))
    {
        return {};
    }
    
    DynamicLiveResource imGuiImage
    {
        .type = DynamicResourceType::Image,
        .resource = std::make_unique<Image>(
            m_allocator.Get(),
            *m_commandPool,
            *m_context,
            texRes.Get()->GetPixelData().data(),
            texRes.Get()->GetPixelData().size(),
            texRes.Get()->GetWidth(),
            texRes.Get()->GetHeight(),
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    GUID imageGuid { GuidUtils::Generate() };
    m_liveObjects[imageGuid] = std::move(imGuiImage);
    
    DynamicResourceHandle<Image> imGuiImageHandle { imageGuid };
    
    DynamicLiveResource imGuiImageView 
    {
        .type = DynamicResourceType::ImageView,
        .resource = std::make_unique<ImageView>(
            m_context->GetDevice(),
            imGuiImageHandle->Get(),
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageAspectFlagBits::eColor)
    };
    
    GUID imageViewGuid { GuidUtils::Generate() };
    m_liveObjects[imageViewGuid] = std::move(imGuiImageView);
    
    DynamicResourceHandle<ImageView> imGuiImageViewHandle { imageViewGuid }; 
    
    DynamicLiveResource imGuiTexture 
    {
        .type = DynamicResourceType::ImGuiTexture,
        .resource = std::make_unique<ImGuiTextureResource>(
            ImGui_ImplVulkan_AddTexture(
                *m_textureSampler.Get(),
                *imGuiImageViewHandle->Get(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            )
        )
    };
    
    GUID textureGuid { GuidUtils::Generate() };
    m_liveObjects[textureGuid] = std::move(imGuiTexture);
    
    DynamicResourceHandle<ImGuiTextureResource> imGuiTextureResourceHandle { textureGuid };
    
    return ImGuiTexture 
    {
        imGuiTextureResourceHandle,
        imGuiImageHandle,
        imGuiImageViewHandle
    };
}

void IntermediaryStorage::UpdateImGuiRenderTarget(RenderTarget p_target, ImGuiTexture p_texture, std::uint32_t p_width, std::uint32_t p_height)
{
    ZoneScoped;
    
    // Create Colour Image
    
    GUID colourImageGuid { p_target.GetColourImageGuid() };
    DynamicLiveResource colourImage 
    {
        .type = DynamicResourceType::Image,
        .refCount = m_liveObjects[colourImageGuid].refCount,
        .resource = std::make_unique<Image>(m_allocator.Get(),
            p_width,
            p_height,
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal),
    };
    
    MoveToDestructionQueue(colourImageGuid);
    m_liveObjects[colourImageGuid] = std::move(colourImage);
    
    DynamicResourceHandle<Image> colourImageHandle { colourImageGuid };
    
    // Create Colour Image View
    
    GUID colourImageViewGuid { p_target.GetColourImageViewGuid() };
    DynamicLiveResource colourImageView 
    {
        .type = DynamicResourceType::ImageView,
        .refCount = m_liveObjects[colourImageViewGuid].refCount,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
            colourImageHandle->Get())       
    };
    
    MoveToDestructionQueue(colourImageViewGuid);
    m_liveObjects[colourImageViewGuid] = std::move(colourImageView);
    
    // Create Depth Image
    
    GUID depthImageGuid { p_target.GetDepthImageGuid() };
    DynamicLiveResource depthImage
    {
        .type = DynamicResourceType::Image,
        .refCount = m_liveObjects[depthImageGuid].refCount,
        .resource = std::make_unique<Image>(m_allocator.Get(),
        p_width,
        p_height,
        vk::Format::eD32Sfloat, 
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    
    MoveToDestructionQueue(depthImageGuid);
    m_liveObjects[depthImageGuid] = std::move(depthImage);
    
    DynamicResourceHandle<Image> depthImageHandle { depthImageGuid };
    
    // Create Depth Image View
    
    GUID depthImageViewGuid { p_target.GetDepthImageViewGuid() };
    DynamicLiveResource depthImageView 
    {
        .type = DynamicResourceType::ImageView,
        .refCount = m_liveObjects[depthImageViewGuid].refCount,
        .resource = std::make_unique<ImageView>(m_context->GetDevice(),
        depthImageHandle->Get(),
        vk::Format::eD32Sfloat,
        vk::ImageAspectFlagBits::eDepth)
    };
    
    MoveToDestructionQueue(depthImageViewGuid);
    m_liveObjects[depthImageViewGuid] = std::move(depthImageView);
    
    // Create ImGui Texture
    
    GUID imGuiTextureGuid { p_texture.GetGUID() };
    DynamicLiveResource imGuiTexture
    {
        .type = DynamicResourceType::ImGuiTexture,
        .refCount = m_liveObjects[imGuiTextureGuid].refCount, 
        .resource = std::make_unique<ImGuiTextureResource>(ImGui_ImplVulkan_AddTexture(
            *m_textureSampler.Get(),
            *p_target.GetColourImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        )
    };
    
    MoveToDestructionQueue(imGuiTextureGuid);
    m_liveObjects[imGuiTextureGuid] = std::move(imGuiTexture);
}

Pipeline *IntermediaryStorage::GetPipeline(RenderData p_renderData, vk::SurfaceFormatKHR p_surfaceFormat)
{
    ZoneScoped;
    // TRY FINDING ALREADY CREATED PIPELINE
    
    PipelineKey key {
        .vsGuid = p_renderData.vsData.GUID,
        .tcsGuid = p_renderData.tcsData.GUID,
        .tesGuid = p_renderData.tesData.GUID,
        .gsGuid = p_renderData.gsData.GUID,
        .fsGuid = p_renderData.fsData.GUID
    };
    
    auto it = m_pipelineMap.find(key);
    if (it != m_pipelineMap.end())
    {
        return &it->second;
    }

    // IF FAILED, CREATE NEW
    ResourceManager& RMInstance = ResourceManager::GetInstance();
    StaticResourceHandle<ShaderResource> vs = RMInstance.LoadResource<ShaderResource>(key.vsGuid);
    StaticResourceHandle<ShaderResource> tcs = RMInstance.LoadResource<ShaderResource>(key.tcsGuid);
    StaticResourceHandle<ShaderResource> tes = RMInstance.LoadResource<ShaderResource>(key.tesGuid);
    StaticResourceHandle<ShaderResource> gs = RMInstance.LoadResource<ShaderResource>(key.gsGuid);
    StaticResourceHandle<ShaderResource> fs = RMInstance.LoadResource<ShaderResource>(key.fsGuid);

    if (!(vs.IsReady() || vs.HasFailed()))
    {
		return nullptr;
    }

    std::vector<std::pair<vk::ShaderModule, vk::ShaderStageFlagBits>> shaderVec{};

    // Vertex Shader

    ShaderModule vsModule{ nullptr };
    if (vs.IsReady())
    {
        ShaderResource* vsRes = vs.Get();
        vsModule = ShaderModule{ m_context->GetDevice(), vsRes->GetByteCode() };
        shaderVec.emplace_back(vsModule.Get(), vk::ShaderStageFlagBits::eVertex);
    }
    else if (vs.IsValid())
    {
        return nullptr;
    }
    
    // Tessellation Control Shader

    ShaderModule tcsModule{ nullptr };
    if (tcs.IsReady())
    {
        ShaderResource* tcsRes = tcs.Get();
        tcsModule = ShaderModule{ m_context->GetDevice(), tcsRes->GetByteCode() };
        shaderVec.emplace_back(tcsModule.Get(), vk::ShaderStageFlagBits::eTessellationControl);
    }
    else if (tcs.IsValid())
    {
        return nullptr;
    }
    
    // Tessellation Evaluation Shader

    ShaderModule tesModule{ nullptr };
    if (tes.IsReady())
    {
        ShaderResource* tesRes = tes.Get();
        tesModule = ShaderModule{ m_context->GetDevice(), tesRes->GetByteCode() };
        shaderVec.emplace_back(tesModule.Get(), vk::ShaderStageFlagBits::eTessellationEvaluation);
    }
    else if (tes.IsValid())
    {
        return nullptr;
    }

    // Geometry Shader
    
    ShaderModule gsModule{ nullptr };
    if (gs.IsReady())
    {
        ShaderResource* gsRes = gs.Get();
        gsModule = ShaderModule{ m_context->GetDevice(), gsRes->GetByteCode() };
        shaderVec.emplace_back(gsModule.Get(), vk::ShaderStageFlagBits::eGeometry);
    }
    else if (gs.IsValid())
    {
        return nullptr;
    }

    // Fragment Shader
    
    ShaderModule fsModule{ nullptr };
    if (fs.IsReady())
    {
        ShaderResource* fsRes = fs.Get();
        fsModule = ShaderModule{ m_context->GetDevice(), fsRes->GetByteCode() };
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

    m_pipelineMap[key] = Pipeline{
        m_context->GetDevice(), 
        m_context->GetPhysicalDevice(),
        descriptorSetLayouts,
        shaderVec,
        p_surfaceFormat
    };
    
    return &m_pipelineMap.at(key);
}

const Mesh IntermediaryStorage::GetMesh(const vk::raii::CommandBuffer &p_commandBuffer, RenderData p_renderData)
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
        StaticResourceHandle<MeshResource> meshRes = RMInstance.LoadResource<MeshResource>(p_renderData.meshData.GUID);

        if (!(meshRes.IsReady() || meshRes.HasFailed()))
        {
            Mesh m;
            m.valid = false;
            return m;
        }

        m_vertexMap[key] = VertexBuffer { m_allocator.Get(), m_context->GetDevice(), p_commandBuffer, meshRes.Get()->GetVertexData() };
        m_indexMap[key] = IndexBuffer { m_allocator.Get(), m_context->GetDevice(), p_commandBuffer, meshRes.Get()->GetIndexData() };

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
    StaticResourceHandle<Texture2DResource> texRes = RMInstance.LoadResource<Texture2DResource>(p_renderData.texData.GUID);

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

std::uint32_t IntermediaryStorage::CreatePresentTarget(vk::Image p_presentImage, vk::Format p_format, std::uint32_t p_width, std::uint32_t p_height)
{
    m_presentTargets.emplace_back(
        m_allocator.Get(),
        m_context->GetDevice(),
        m_context->GetPhysicalDevice(),
        p_presentImage,
        p_format,
        p_width,
        p_height);
    
    return static_cast<std::uint32_t>(m_presentTargets.size() - 1);
}
