#include "NodeInspectorWindow.hpp"

using namespace Droplet::Editor;

Droplet::Editor::NodeInspectorWindow::NodeInspectorWindow(std::shared_ptr<InteractionState> p_interactionState)
{
	m_interactionState = p_interactionState;
	//m_currentNode = m_interactionState.get()->GetSelectedNodes()[0];
}

void NodeInspectorWindow::SetCurrentNode(std::shared_ptr<Droplet::Scene::Node> p_nodeToInspect)
{
	m_currentNode = p_nodeToInspect;
}

void NodeInspectorWindow::InitImpl()
{
	// Initialization for the node inspector window
}

void NodeInspectorWindow::CloseImpl()
{
	// Cleanup for the node inspector window
}

void NodeInspectorWindow::RenderImpl()
{
	ImGui::Begin("Node Inspector");

	static std::string stringNodeName = "Not a real node :P";
	ImGui::InputText("Node Name", &stringNodeName);

	static bool active = true;
	ImGui::Checkbox("Active", &active);

	ImGui::End();

	//Name (which should be able to change)
	//Active checkbox
	//Transform
	//List of components (Do i need a component list with imgui functions for this?)
}

void NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
