#include <Graphics/VK/SceneAdapter.hpp>
#include <Graphics/VK/TestData.hpp> 

using namespace Droplet::Graphics::VK;

void SceneAdapter::Initialize(const Context &p_context, const CommandPool &p_commandPool, const Allocator &p_allocator, DescriptorSet &p_descriptorSet)
{
	m_vertexMap.emplace(0, VertexBuffer(p_allocator.Get(), G_VERTICES));
	m_indexMap.emplace(0, IndexBuffer(p_allocator.Get(), G_INDICES));
	
	m_samplerMap.emplace(0, Sampler(p_context));

	m_imageMap.emplace(0, 
		Image(
			p_allocator.Get(),
			p_commandPool,
			p_context,
			G_CATDESPAIR,
			78400,
			G_CATDIM,
			G_CATDIM,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		)
	);

	m_viewMap.emplace(0,
		ImageView(
			p_context.GetDevice(),
			m_imageMap[0].Get(),
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageAspectFlagBits::eColor
		)
	);

	p_descriptorSet.WriteCombinedImageSamplerDescriptor(p_context.GetDevice(), m_samplerMap[0].Get(), m_viewMap[0].Get());
}

void SceneAdapter::UpdateMaps(const Context &p_context, const Allocator &p_allocator, DescriptorSet &p_descriptorSet)
{
	//Iterate through and update what meshes and textures are loaded

	m_renderData.meshData.clear();
	m_renderData.pushConstants.clear();

	std::vector<MeshData> mData;
	std::vector<PushConstants> pData;
	for (size_t i = 0; i < m_nodeData.size(); i++)
	{
		MeshData m;
		m.indexBuffer = m_indexMap[m_nodeData[i].meshID].Get();
		m.vertexBuffer = m_vertexMap[m_nodeData[i].meshID].Get();
		m.indexCount = m_indexMap[m_nodeData[i].meshID].GetIndexCount();

		mData.push_back(m);

		m_bufferMap.emplace(i, StorageBuffer(p_allocator.Get(), m_nodeData[i].world));
		p_descriptorSet.WriteStorageBufferDescriptor(p_context.GetDevice(), m_bufferMap[i].Get(), i + 1);

		PushConstants p;
		p.unibuf = i;
		p.sampler = 0;

		pData.push_back(p);
	}

	m_renderData.meshData = mData;
	m_renderData.pushConstants = pData;
}

void SceneAdapter::SubmitNodeData(const std::vector<Droplet::Scene::NodeData> p_nodeData)
{
	m_nodeData = p_nodeData;
}


const RenderData SceneAdapter::GetRenderData()
{
	return m_renderData;
}