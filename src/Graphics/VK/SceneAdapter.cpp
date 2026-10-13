#include <Graphics/VK/SceneAdapter.hpp>

using namespace Droplet::Graphics::VK;

SceneAdapter::SceneAdapter(const Context &p_context) :
	m_context(p_context)
{
}

void SceneAdapter::UpdateMaps(const std::vector<Droplet::Scene::NodeData> p_nodeData)
{
	for (auto &it : m_meshMap)
	{

	}

	for (auto  &it : m_textureMap)
	{

	}
}

const RenderData SceneAdapter::GetRenderData()
{
	RenderData data
	{

	};

	return data;
}