#include "NodeInspectorWindow.hpp"
#include <SceneSystem/Component.hpp>
#include <SceneSystem/ComponentFactory.hpp>
#include <algorithm>
#include <TransformUI.hpp>
#include <ComponentList.hpp>
#include <Editor/ImGuiUtils.hpp>
#include <EditorConfig.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>

using namespace Droplet::Editor;

NodeInspectorWindow::NodeInspectorWindow(EditorContext *p_editorContext) 
	: m_editorContext(p_editorContext)
{ }

void NodeInspectorWindow::SetCurrentNode(std::shared_ptr<Droplet::Scene::Node> p_nodeToInspect) // TODO: Make weak pointer
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
	const auto &selectedNodes = m_editorContext->GetInteractionState().GetSelectedNodes();

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
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		using namespace Droplet::Scene;
		using TSpace = Transform::Space;

		TSpace space = (TSpace)EditorConfig::LoadInt("manipulateSpace", static_cast<int>(TSpace::World));

		Transform &transform = m_currentNode->GetTransform();
		float drag_speed = 0.005f;

		glm::vec3 pos = transform.GetPosition(space);
		if (ImGui::DragFloat3("Position", &pos.x, drag_speed))
		{
			transform.SetPosition(pos, space);
		}
		ImGui::LockToItem();

		// Displayed angles are only a readout; edits are applied as per-axis delta rotations
		glm::vec3 &rot = m_displayedRotation;
		if (!m_rotationActive)
		{
			rot = glm::degrees(transform.GetEuler(space));
		}
		glm::vec3 prevRot = rot;
		if (ImGui::DragFloat3("Rotation", &rot.x, drag_speed * 40.0f))
		{
			glm::vec3 normal = glm::vec3(0, 1, 0);
			float deltaRot = 0.0f;

			if (rot.x != prevRot.x)
			{
				normal = (space == TSpace::Local) ? transform.GetRight() : glm::vec3(1, 0, 0);
				deltaRot = glm::radians(rot.x - prevRot.x);
			}
			else if (rot.y != prevRot.y)
			{
				normal = (space == TSpace::Local) ? transform.GetUp() : glm::vec3(0, 1, 0);
				deltaRot = glm::radians(rot.y - prevRot.y);
			}
			else if (rot.z != prevRot.z)
			{
				normal = (space == TSpace::Local) ? transform.GetForward() : glm::vec3(0, 0, 1);
				deltaRot = glm::radians(rot.z - prevRot.z);
			}

			glm::quat deltaQuat = glm::angleAxis(deltaRot, normal);
			transform.Rotate(deltaQuat, space);
		}
		ImGui::LockToItem();

		m_rotationActive = ImGui::IsItemActive();

		glm::vec3 scl = transform.GetScale();
		const glm::vec3 oldScl = scl;
		if (ImGui::DragFloat3("Scale", &scl.x, drag_speed * 0.5f))
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
		ImGui::LockToItem();

		// Lock scale
		ImGui::Checkbox("Lock Scale", &m_lockScale);
	}

	// Button width
	const float buttonWidth = ImGui::GetContentRegionAvail().x;

	// Vector of components
	const std::vector<std::shared_ptr<Droplet::Scene::Component>> components = m_currentNode->GetAllComponents(); // TODO: Make weak pointer

	for (const auto &component : components)
	{
		if (!component)
		{
			continue;
		}

		uintptr_t thisNumber = (uintptr_t)(component.get());
		std::string thisString = std::to_string(thisNumber);
		ImGui::PushID(thisString.c_str());

		// The actual header
		bool isOpen = ImGui::CollapsingHeader(component->GetTypeName().data(), ImGuiTreeNodeFlags_DefaultOpen);

		// The right click on header
		if (ImGui::BeginPopupContextItem("popup"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				m_currentNode->RemoveComponent(component.get());
			}
			ImGui::EndPopup();
		}

		// Rendering the component UI if header is open
		if (isOpen)
		{
			component->RenderUI();
		}

		ImGui::PopID();
	}

	ImGui::Spacing();

	if (ImGui::Button("+ Add Component", ImVec2(buttonWidth, 0.0f)))
	{
		Scene::OpenComponentList();
	}

	std::string selectedComponentName = "";
	if (Scene::ShowComponentList(&selectedComponentName))
	{
		auto newComp = Droplet::Scene::ComponentFactory::CreateComponent(selectedComponentName);
		//HACK: SHOULD NOT DO THIS
		if (selectedComponentName == "CameraComponent")
		{
			auto cameraComponent =
				dynamic_cast<Droplet::Scene::CameraComponent *>(newComp.get());

			if (cameraComponent)      
			{
				cameraComponent->SetCamera(&m_editorContext->GetEditorCamera());
			}
		}
		m_currentNode->AttachComponent(newComp);
	}

	ImGui::PopID();
}

void NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
