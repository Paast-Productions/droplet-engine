#include "NodeInspectorWindow.hpp"
#include <SceneSystem/Component.hpp>
#include <SceneSystem/ComponentFactory.hpp>
#include <algorithm>
#include <ComponentList.hpp>
#include <Editor/ImGuiUtils.hpp>
#include <EditorConfig.hpp>
#include <TransformSnapping.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <Editor/DragDropFields.hpp>
#include <cmath>

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
		m_currentNode.lock()->SetActiveSelf(active);
	}

	Common::DrawLayer drawLayer = m_currentNode.lock()->GetDrawLayer();
	if (Common::DrawLayerUtility::SetLayerUI("Draw Layer", drawLayer))
	{
		m_currentNode.lock()->SetDrawLayer(drawLayer);
	}

	CollisionLayer collisionLayerMask = m_currentNode.lock()->GetCollisionLayerMask();
	if (CollisionLayerUtility::SetLayerMaskUI("Collision Layers", collisionLayerMask))
	{
		m_currentNode.lock()->SetCollisionLayerMask(collisionLayerMask);
	}

	if (m_currentNode.lock()->IsPrefab())
	{
		std::string prefabPath = m_currentNode.lock()->GetPrefabPath();

		float undoPrefabButtonPosX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - iconButtonWidth;

		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.1f, 1.0f), "Prefab: %s", prefabPath.c_str());

		ImGui::SameLine();

		ImGui::SetCursorPosX(undoPrefabButtonPosX);

		// Make red
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.3f, 0.3f, 1.0f));
		
		if (ImGui::Button(ICON_LC_X))
		{
			m_currentNode.lock()->SetPrefabPath("");
		}

		ImGui::PopStyleColor(3);
	}

	// Transform
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		using namespace Droplet::Scene;
		using TSpace = Transform::Space;

		TSpace space = (TSpace)EditorConfig::LoadInt("manipulateSpace", static_cast<int>(TSpace::World));
		const TransformSnapSettings snapSettings = TransformSnapSettings::Load();
		const bool snapActive = snapSettings.IsActive(ImGui::GetIO().KeyAlt);
		const float translateSnap = snapActive ? snapSettings.translate : 0.0f;
		const float rotateSnap = snapActive ? snapSettings.rotate : 0.0f;
		const float scaleSnap = snapActive ? snapSettings.scale : 0.0f;

		Transform &transform = m_currentNode.lock()->GetTransform();
		float drag_speed = 0.005f;

		glm::vec3 pos = transform.GetPosition(space);
		const bool snapRelative = !snapSettings.absolute;
		if (DragFloat3Snapped("Position", pos, drag_speed, translateSnap, snapRelative))
		{
			transform.SetPosition(pos, space);
		}
		ImGui::LockToItem();

		if (ImGui::BeginPopupContextItem("Position Context Menu"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				transform.SetPosition(glm::vec3(0.0f), space);
			}
			ImGui::EndPopup();
		}

		glm::vec3 prevRot = glm::degrees(transform.GetEuler(space));
		glm::vec3 rot = prevRot;
		if (DragFloat3Snapped("Rotation", rot, drag_speed * 40.0f, rotateSnap, snapRelative))
		{
			/*if (rot.x != prevRot.x)
			{
				float deltaRot = glm::radians(rot.x - prevRot.x);
				transform.RotateAxis(deltaRot, glm::vec3(1.0f, 0.0f, 0.0f), space);
			}
			else if (rot.y != prevRot.y)
			{
				float deltaRot = glm::radians(rot.y - prevRot.y);
				transform.RotateAxis(deltaRot, glm::vec3(0.0f, 1.0f, 0.0f), space);
			}
			else if (rot.z != prevRot.z)
			{
				float deltaRot = glm::radians(rot.z - prevRot.z);
				transform.RotateAxis(deltaRot, glm::vec3(0.0f, 0.0f, 1.0f), space);
			}*/
			transform.SetEuler(glm::radians(rot), space);
		}
		ImGui::LockToItem();

		// HACK: Resetting rotation to zero doesn't consistently undo all rotation. 
		// Added a right-click context menu to reset rotation to alleviate issue.

		if (ImGui::BeginPopupContextItem("Rotation Context Menu"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				transform.SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), space);
			}
			ImGui::EndPopup();
		}

		glm::vec3 scl = transform.GetScale();
		const glm::vec3 oldScl = scl;
		int scaleAxis = -1;
		if (DragFloat3Snapped("Scale", scl, drag_speed * 0.5f, scaleSnap, snapRelative, &scaleAxis))
		{
			if (m_lockScale && scaleAxis >= 0 && oldScl[scaleAxis] != 0.0f)
			{
				// Other axes keep their ratio to the edited axis, which is already snapped.
				const float factor = scl[scaleAxis] / oldScl[scaleAxis];
				for (int i = 0; i < 3; ++i)
				{
					if (i != scaleAxis)
					{
						scl[i] = oldScl[i] * factor;
					}
				}
			}

			transform.SetScale(scl);
		}
		ImGui::LockToItem();

		if (ImGui::BeginPopupContextItem("Scale Context Menu"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				transform.SetScale(glm::vec3(1.0f));
			}
			ImGui::EndPopup();
		}

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

bool NodeInspectorWindow::DragFloat3Snapped(const char *p_label, glm::vec3 &p_value, float p_speed, float p_snapIncrement, bool p_relative, int *p_changedAxis)
{
	if (p_changedAxis)
	{
		*p_changedAxis = -1;
	}

	const ImGuiID id = ImGui::GetID(p_label);
	const bool dragging = m_snapDrag.id == id;
	glm::vec3 raw = dragging ? m_snapDrag.value : p_value;
	const glm::vec3 start = dragging ? m_snapDrag.start : p_value;

	const glm::vec3 previousRaw = raw;
	const bool changed = ImGui::DragFloat3(p_label, &raw.x, p_speed);

	if (ImGui::IsAnyItemActive())
	{
		if (changed || dragging)
		{
			m_snapDrag = { id, raw, start };
		}
	}
	else
	{
		m_snapDrag.id = 0;
	}

	if (!changed)
	{
		return false;
	}

	// Only the axis being edited is written, so untouched axes keep their values.
	const bool snap = p_snapIncrement > 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		if (raw[i] == previousRaw[i])
		{
			continue;
		}
		if (p_changedAxis && *p_changedAxis < 0)
		{
			*p_changedAxis = i;
		}
		const float origin = p_relative ? start[i] : 0.0f;
		p_value[i] = snap ? SnapToIncrement(raw[i], p_snapIncrement, origin) : raw[i];
	}
	return true;
}
