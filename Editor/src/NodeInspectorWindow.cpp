#include "NodeInspectorWindow.hpp"
#include <SceneSystem/Component.hpp>
#include <algorithm>
#include <TransformUI.hpp>

using namespace Droplet::Editor;

Droplet::Editor::NodeInspectorWindow::NodeInspectorWindow(std::shared_ptr<InteractionState> p_interactionState)
{
	m_interactionState = p_interactionState;
}

void NodeInspectorWindow::SetCurrentNode(std::shared_ptr<Droplet::Scene::Node> p_nodeToInspect)
{
	m_currentNode = p_nodeToInspect;
}

void NodeInspectorWindow::InitImpl()
{
	// Initialization for the node inspector window
	SetName("Node Inspector");
}

void NodeInspectorWindow::CloseImpl()
{
	// Cleanup for the node inspector window
}

void NodeInspectorWindow::RenderImpl()
{
	const auto &selectedNodes = m_interactionState->GetSelectedNodes();

	// No selected node
	if (selectedNodes.empty())
	{
		ImGui::Text("Select a node");
		return;
	}

	m_currentNode = selectedNodes.back();

	// Name
	std::string stringNodeName = m_currentNode->GetName();
	if (ImGui::InputText("Node Name", &stringNodeName))
	{
		m_currentNode->SetName(stringNodeName);
	}

	// Active
	bool active = m_currentNode->IsActiveSelf();
	if (ImGui::Checkbox("Active", &active))
	{
		m_currentNode->SetActive(active);
	}

	// Transform
	if (ImGui::CollapsingHeader("Transform"))
	{
		Droplet::Scene::Transform &transform = m_currentNode->GetTransform();
		float drag_speed = 0.001f;

		glm::vec3 pos = transform.GetPosition();
		if (ImGui::DragFloat3("Position", &pos.x, drag_speed))
		{
			transform.SetPosition(pos);
		}

		glm::vec3 rot = transform.GetEuler();
		if (ImGui::DragFloat3("Rotation", &rot.x, drag_speed))
		{
			transform.SetEuler(rot);
		}

		glm::vec3 scl = transform.GetScale();
		glm::vec3 oldScl = transform.GetScale();
		if (ImGui::DragFloat3("Scale", &scl.x, drag_speed))
		{
			if (m_lockScale)
			{
				if (scl.x != oldScl.x)
				{
					float deltaScale = scl.x - oldScl.x;
					scl.y += deltaScale * (oldScl.y / oldScl.x);
					scl.z += deltaScale * (oldScl.z / oldScl.x);
				}
				else if (scl.y != oldScl.y)
				{
					float deltaScale = scl.y - oldScl.y;
					scl.x += deltaScale * (oldScl.x / oldScl.y);
					scl.z += deltaScale * (oldScl.z / oldScl.y);
				}
				else if (scl.z != oldScl.z)
				{
					float deltaScale = scl.z - oldScl.z;
					scl.x += deltaScale * (oldScl.x / oldScl.z);
					scl.y += deltaScale * (oldScl.y / oldScl.z);
				}
			}
			transform.SetScale(scl);
		}

		// Lock scale
		ImGui::Checkbox("Lock Scale", &m_lockScale);
	}

	// Vector of components
	const std::vector<std::shared_ptr<Droplet::Scene::Component>> components = m_currentNode->GetAllComponents();

	for (const auto &component : components)
	{
		if (!component)
		{
			continue;
		}
		component->RenderUI();
	}

	//List of components (Do i need a component list with imgui functions for this?)
	//en add+ knapp ska läggas till som visar alla potentiella components, så endast då behövs en lista
}

void NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
