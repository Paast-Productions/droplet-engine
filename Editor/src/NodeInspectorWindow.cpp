#include "NodeInspectorWindow.hpp"

using namespace Droplet::Editor;

void NodeInspectorWindow::SetCurrentNode(Droplet::Scene::Node *p_nodeToInspect)
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

	float f;
	ImGui::Text("Hi im Node Inspector!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

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
