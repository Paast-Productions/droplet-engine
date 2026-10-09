#include "NodeInspectorWindow.hpp"
#include <SceneSystem/Component.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
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

	// Push ID
	uintptr_t nodeNumber = (uintptr_t)(m_currentNode.get());
	std::string nodeString = std::to_string(nodeNumber);
	ImGui::PushID(nodeString.c_str());

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

		uintptr_t thisNumber = (uintptr_t)(component.get());
		std::string thisString = std::to_string(thisNumber);
		ImGui::PushID(thisString.c_str());

		if (ImGui::CollapsingHeader(component->GetTypeName().data()))
		{
			component->RenderUI();
		}
		ImGui::PopID();
	}

	ImGui::Spacing();

	const float buttonWidth = ImGui::GetContentRegionAvail().x;

	if (ImGui::Button("+ Add Component", ImVec2(buttonWidth, 0.0f)))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		if (ImGui::MenuItem("Script Component"))
		{
			m_currentNode->AddComponent<Scene::ScriptComponent>();
		}

		// TODO: Add for each component we have

		ImGui::EndPopup();
	}

	ImGui::PopID();
	//List of components (Do i need a component list with imgui functions for this?)
}

void NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
