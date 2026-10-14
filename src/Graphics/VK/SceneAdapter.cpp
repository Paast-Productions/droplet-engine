#include <Graphics/VK/SceneAdapter.hpp>
#include <Graphics/VK/TestData.hpp> 

using namespace Droplet::Graphics::VK;

SceneAdapter::SceneAdapter(const Context &p_context, const CommandPool &p_commandPool, const Allocator &p_allocator, DescriptorSet &p_descriptorSet) :
	m_context{ p_context },
	m_commandPool{p_commandPool},
	m_allocator{p_allocator},
	m_descriptorSet{p_descriptorSet}
{
	m_vertexMap.emplace(0, VertexBuffer(m_allocator.Get(), G_VERTICES));
	m_indexMap.emplace(0, IndexBuffer(m_allocator.Get(), G_INDICES));
	
	m_samplerMap.emplace(0, Sampler(m_context));

	m_imageMap.emplace(0, 
		Image(
			m_allocator.Get(),
			m_commandPool,
			m_context,
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
			m_context.GetDevice(),
			m_imageMap[0].Get(),
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageAspectFlagBits::eColor
		)
	);

	m_descriptorSet.WriteCombinedImageSamplerDescriptor(m_context.GetDevice(), m_samplerMap[0].Get(), m_viewMap[0].Get());
}

void SceneAdapter::UpdateMaps(const std::vector<Droplet::Scene::NodeData> p_nodeData)
{
	//Iterate through and update what meshes and textures are loaded

	m_renderData.meshData.clear();
	m_renderData.pushConstants.clear();

	std::vector<MeshData> mData;
	std::vector<PushConstants> pData;
	for (size_t i = 0; i < p_nodeData.size(); i++)
	{
		MeshData m;
		m.indexBuffer = m_indexMap[p_nodeData[i].meshID].Get();
		m.vertexBuffer = m_vertexMap[p_nodeData[i].meshID].Get();
		m.indexCount = m_indexMap[p_nodeData[i].meshID].GetIndexCount();

		mData.push_back(m);

		m_bufferMap.emplace(i, StorageBuffer(m_allocator.Get(), p_nodeData[i].world));
		m_descriptorSet.WriteStorageBufferDescriptor(m_context.GetDevice(), m_bufferMap[i].Get());

		PushConstants p;
		p.unibuf = i;
		p.sampler = 0;

		pData.push_back(p);
	}

	m_renderData.meshData = mData;
	m_renderData.pushConstants = pData;
}

const RenderData SceneAdapter::GetRenderData()
{
	return m_renderData;
}