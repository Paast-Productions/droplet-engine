#include "NodeInspectorWindow.hpp"

void Droplet::Editor::NodeInspectorWindow::InitImpl()
{
	// Initialization for the node inspector window
}

void Droplet::Editor::NodeInspectorWindow::CloseImpl()
{
	// Cleanup for the node inspector window
}

void Droplet::Editor::NodeInspectorWindow::RenderImpl()
{
	ImGui::Begin("Node Inspector");

	float f;
	ImGui::Text("Hi im Node Inspector!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	ImGui::End();
}

void Droplet::Editor::NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
