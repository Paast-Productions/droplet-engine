#include "IntermediaryStorage.hpp"

#include <vector>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyVulkan.hpp>
#include <Graphics/VK/ShaderModule.hpp> 
#include <Debug/Logger.hpp>
#include <ImGui/imgui_impl_vulkan.h>

using namespace Droplet::Graphics::VK;

IntermediaryStorage::IntermediaryStorage(Context &p_context, DescriptorSet &p_descriptorSet) :
    m_context(p_context),
    m_descriptorSet(p_descriptorSet),
    m_allocator(m_context.GetInstance(), m_context.GetPhysicalDevice(), m_context.GetDevice()),
    m_textureSampler(m_context) {}

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

    if (it == m_pipelineMap.end())
    {
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
        if (vs.IsReady() && vs.Get()->GetType() == ShaderResource::ShaderType::Vertex)
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
        if (tcs.IsReady() && tcs.Get()->GetType() == ShaderResource::ShaderType::TessellationControl)
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
        if (tes.IsReady() && tes.Get()->GetType() == ShaderResource::ShaderType::TessellationEvaluation)
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
        if (gs.IsReady() && gs.Get()->GetType() == ShaderResource::ShaderType::Geometry)
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
        if (fs.IsReady() && fs.Get()->GetType() == ShaderResource::ShaderType::Fragment)
        {
            ShaderResource* fsRes = fs.Get();
            fsModule = ShaderModule{ p_device, fsRes->GetByteCode() };
            shaderVec.emplace_back(fsModule.Get(), vk::ShaderStageFlagBits::eFragment);
        }
        else if (fs.IsValid())
        {
            return nullptr;
        }

        m_pipelineMap[key] = Pipeline
        {
            p_device,
            p_physicalDevice,
            m_descriptorSet.GetLayout(),
            shaderVec,
            p_surfaceFormat
        };
        return m_pipelineMap.at(key);
    }
    else
    {
        return m_pipelineMap.at(key);
    }
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

const std::uint32_t IntermediaryStorage::GetTextureID(RenderData p_renderData, CommandPool &p_commandPool, bool &result)
{
    ZoneScoped;

    std::uint64_t guid = p_renderData.texData.GUID;
    

    auto it = m_textureGuidMap.find(guid);

    if (it == m_textureGuidMap.end())
    {
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
        m_viewMap.emplace_back(
            m_context.GetDevice(),
            m_imageMap.back().Get(),
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageAspectFlagBits::eColor
        );

        m_descriptorSet.WriteCombinedImageSamplerDescriptor(m_context.GetDevice(), m_textureSampler.Get(), m_viewMap.back().Get());

        m_textureGuidMap[guid] = m_viewMap.size() - 1;

        result = true;
        return m_textureGuidMap.at(guid);
    }
    else
    {
        result = true;
        return m_textureGuidMap.at(guid);
    }
}

VkDescriptorSet IntermediaryStorage::GetOrCreateImGuiTexture(GUID p_guid, CommandPool &p_commandPool)
{
    ZoneScoped;

    auto it = m_imGuiTexMap.find(p_guid);

    if (it != m_imGuiTexMap.end())
    {
		return m_imGuiTexMap[p_guid].descriptorSet;
    }

    ResourceManager &RMInstance = ResourceManager::GetInstance();
    ResourceHandle<Texture2DResource> texRes = RMInstance.LoadResource<Texture2DResource>(p_guid);

    if (!(texRes.IsReady() || texRes.HasFailed()))
    {
        return VK_NULL_HANDLE;
    }

	ImGuiTexture &imguiTex = m_imGuiTexMap[p_guid];

    imguiTex.image = Image(
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

    imguiTex.view = ImageView(
        m_context.GetDevice(),
        imguiTex.image.Get(),
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageAspectFlagBits::eColor
    );

	imguiTex.descriptorSet = ImGui_ImplVulkan_AddTexture(
        *m_textureSampler.Get(),
        *imguiTex.view.Get(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

	return imguiTex.descriptorSet;
}

void IntermediaryStorage::RemoveImGuiTexture(GUID p_guid)
{
	auto it = m_imGuiTexMap.find(p_guid);
	if (it == m_imGuiTexMap.end())
	{
        return;
	}

	ImGui_ImplVulkan_RemoveTexture(it->second.descriptorSet);
	m_imGuiTexMap.erase(it);
}
