#include "NodeInspectorWindow.hpp"
#include <SceneSystem/Component.hpp>
#include <SceneSystem/ComponentFactory.hpp>
#include <algorithm>
#include <ComponentList.hpp>
#include <Editor/ImGuiUtils.hpp>
#include <EditorConfig.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <Editor/DragDropFields.hpp>

using namespace Droplet::Editor;

void NodeInspectorWindow::SetCurrentNode(std::weak_ptr<Droplet::Scene::Node> p_nodeToInspect)
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

	m_currentNode = selectedNodes.back().lock();

	// Push ID
	uintptr_t nodeNumber = (uintptr_t)(m_currentNode.lock().get());
	std::string nodeString = std::to_string(nodeNumber);
	ImGui::PushID(nodeString.c_str());

	float availWidth = ImGui::GetContentRegionAvail().x;
	float iconButtonWidth = ImGui::GetFrameHeight();

	ImGui::SetNextItemWidth(availWidth - iconButtonWidth - ImGui::GetStyle().ItemSpacing.x);

	// Name
	std::string stringNodeName = m_currentNode.lock()->GetName();
	if (ImGui::InputText("##Node Name", &stringNodeName))
	{
		m_currentNode.lock()->SetName(stringNodeName);
	}

	ImGui::SameLine();

	// Active
	bool active = m_currentNode.lock()->IsActiveSelf();
	if (ImGui::Checkbox("##Active", &active))
	{
		m_currentNode.lock()->SetActive(active);
	}

	Graphics::DrawLayer drawLayer = m_currentNode.lock()->GetDrawLayer();
	if (Graphics::DrawLayerUtility::SetLayerUI("Draw Layer", drawLayer))
	{
		m_currentNode.lock()->SetDrawLayer(drawLayer);
	}

	// Transform
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		using namespace Droplet::Scene;
		using TSpace = Transform::Space;

		TSpace space = (TSpace)EditorConfig::LoadInt("manipulateSpace", static_cast<int>(TSpace::World));

		Transform &transform = m_currentNode.lock()->GetTransform();
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
	const float buttonHeight = ImGui::GetTextLineHeight() * 1.5f;
	const float buttonMargin = buttonWidth * 0.2f;

	// Vector of components
	const std::vector<std::shared_ptr<Droplet::Scene::Component>> components = m_currentNode.lock()->GetAllComponents(); // TODO: Make weak pointer

	for (std::size_t componentIndex = 0; componentIndex < components.size(); ++componentIndex)
	{
		const auto &component = components[componentIndex];
		if (!component)
		{
			continue;
		}

		uintptr_t thisNumber = (uintptr_t)(component.get());
		std::string thisString = std::to_string(thisNumber);
		ImGui::PushID(thisString.c_str());

		// The actual header
		std::string headerLabel = "";

		if (component->GetTypeName() == "ScriptComponent")
		{
			// Set name to the script name if it has a script
			Droplet::Scene::ScriptComponent *scriptComp = dynamic_cast<Droplet::Scene::ScriptComponent *>(component.get());
			std::string scriptPath = scriptComp->GetScriptPath();

			if (!scriptPath.empty())
			{
				std::string scriptName = scriptPath.substr(scriptPath.find_last_of("/\\") + 1);
				scriptName = scriptName.substr(0, scriptName.find_last_of('.'));
				headerLabel = scriptName + " (script)";
			}
		}
		
		if (headerLabel.empty())
		{
			headerLabel = component->GetTypeName().data();

			// Remove "component" suffix from name if it exists
			if (headerLabel.size() >= 9 && headerLabel.compare(headerLabel.size() - 9, 9, "Component") == 0)
			{
				headerLabel = headerLabel.substr(0, headerLabel.size() - 9);
			}
		}

		bool isOpen = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		if (ImGui::BeginDragDropSource())
		{
			ComponentDragDropData payloadData{m_currentNode.lock().get(), component.get()};
			ImGui::SetDragDropPayload(PAYLOAD_COMPONENT.data(), &payloadData, sizeof(payloadData));
			ImGui::TextUnformatted(headerLabel.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PAYLOAD_COMPONENT.data()))
			{
				if (payload->DataSize == sizeof(ComponentDragDropData))
				{
					const auto &data = *static_cast<const ComponentDragDropData *>(payload->Data);
					auto sourceNode = data.sourceNode
						? data.sourceNode->weak_from_this().lock()
						: nullptr;
					if (sourceNode && data.component)
					{
						sourceNode->MoveComponent(
							data.component,
							*m_currentNode.lock(),
							componentIndex);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		// The right click on header
		if (ImGui::BeginPopupContextItem("popup"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				m_currentNode.lock()->RemoveComponent(component.get());
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

	ImGui::Dummy(ImVec2(0, 18));

	ImGui::Indent(buttonMargin);
	if (ImGui::Button("+ Add Component", ImVec2(buttonWidth - buttonMargin * 2.0f, buttonHeight)))
	{
		Scene::OpenComponentList();
	}

	std::string selectedComponentName = "";
	if (Scene::ShowComponentList(&selectedComponentName))
	{
		auto newComp = Droplet::Scene::ComponentFactory::CreateComponent(selectedComponentName);
		m_currentNode.lock()->AttachComponent(newComp);
	}

	ImGui::PopID();
}

void NodeInspectorWindow::RenderToolbar()
{
	// Toolbar will render here
}
