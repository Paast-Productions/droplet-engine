#include <InteractionState.hpp>

using namespace Droplet::Editor;

void InteractionState::SelectNode(const std::shared_ptr<Droplet::Scene::Node> &p_node)
{
	m_selectedNodes.clear();
	m_selectedNodes.emplace_back(p_node);
}