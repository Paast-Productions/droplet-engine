#ifndef _GAME
#include "Editor/NodeSlot.hpp"
#include <SceneSystem/Node.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Editor;

bool Droplet::Editor::NodeSlot(
	const char *p_label, 
	std::weak_ptr<Node> &p_node,
	ImVec2 p_buttonSize, 
	std::optional<std::function<bool(Droplet::Scene::Node*)>> p_acceptCallback,
	std::optional<std::function<void()>> p_onClickCallback,
	const char *p_noAcceptMsg)
{
    using namespace ImGui;

	PushID(p_label);
    BeginGroup();

	Text(p_label);

    SameLine();

	if (p_buttonSize.x > 0)
	{
		float labelWidth = CalcTextSize(p_label).x;
		labelWidth += GetStyle().ItemSpacing.x;
		p_buttonSize.x = std::max(24.0f, p_buttonSize.x - labelWidth);
	}

	std::string displayName = "None";
	if (auto nodePtr = p_node.lock())
	{
		displayName = nodePtr->GetName();
	}

	if (Button(displayName.c_str(), p_buttonSize))
	{
		if (p_onClickCallback.has_value())
		{
			(p_onClickCallback.value())();
		}
	}

	bool didDrop = false;

	if (BeginDragDropTarget())
	{
		bool acceptDrop = true;
		if (p_acceptCallback.has_value())
		{
			const ImGuiPayload *payload = GetDragDropPayload();

			if (payload && payload->DataSize == sizeof(Node *))
			{
				Node *droppedNode = *static_cast<Node **>(payload->Data);
				acceptDrop = (p_acceptCallback.value())(droppedNode);
			}
		}

		if (acceptDrop)
		{
			if (const ImGuiPayload *payload = AcceptDragDropPayload("SCENE_NODE"))
			{
				if (payload->DataSize == sizeof(Node *))
				{
					Node *droppedNode = *static_cast<Node **>(payload->Data);
					p_node = droppedNode->shared_from_this();
					didDrop = true;
				}
			}
		}
		else
		{
			if (BeginTooltip())
			{
				TextUnformatted(p_noAcceptMsg);
				EndTooltip();
			}
		}

		EndDragDropTarget();
	}

	// Clear-popup on right-click
	if (IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right))
	{
		OpenPopup("NodeSlotPopup");
	}

	if (BeginPopup("NodeSlotPopup"))
	{
		if (Button("Clear"))
		{
			p_node.reset();
			didDrop = true;
			CloseCurrentPopup();
		}

		EndPopup();
	}

    EndGroup();
    PopID();

    return didDrop;
}

bool Droplet::Editor::NodeListSlot(
	const char *p_label, 
	std::vector<std::weak_ptr<Droplet::Scene::Node>> &p_nodes,
	int p_maxNodes, 
	std::optional<std::function<bool(Droplet::Scene::Node *)>> p_acceptCallback, 
	std::optional<std::function<void(int)>> p_onClickCallback, 
	const char *p_noAcceptMsg)
{
	using namespace ImGui;

	PushID(p_label);
	BeginGroup();

	BeginGroup();

	ImGuiStorage *storage = GetStateStorage();

	Text(p_label);

	SameLine();

	ImGuiID expandID = GetID("NodeListSlotExpanded");
	bool isExpanded = storage->GetBool(expandID, false);

	// Expand/Collapse button (up/down arrow)
	if (SmallButton(isExpanded ? ICON_LC_ARROW_BIG_DOWN : ICON_LC_ARROW_BIG_UP))
	{
		isExpanded = !isExpanded;
		storage->SetBool(expandID, isExpanded);
	}

	SameLine();
	TextDisabled(" (%zu items)", p_nodes.size());

	EndGroup();

	// Drag-drop target for appending nodes to the list
	if (BeginDragDropTarget())
	{
		bool acceptDrop = true;
		if (p_acceptCallback.has_value())
		{
			const ImGuiPayload *payload = GetDragDropPayload();
			if (payload && payload->DataSize == sizeof(Node *))
			{
				Node *droppedNode = *static_cast<Node **>(payload->Data);
				acceptDrop = (p_acceptCallback.value())(droppedNode);
			}
		}

		if (acceptDrop)
		{
			if (const ImGuiPayload *payload = AcceptDragDropPayload("SCENE_NODE"))
			{
				if (payload->DataSize == sizeof(Node *))
				{
					Node *droppedNode = *static_cast<Node **>(payload->Data);
					p_nodes.push_back(droppedNode->shared_from_this());
				}
			}
		}
		else
		{
			if (BeginTooltip())
			{
				TextUnformatted(p_noAcceptMsg);
				EndTooltip();
			}
		}
		EndDragDropTarget();
	}

	if (isExpanded)
	{
		SameLine();
		SetNextItemOpen(true, ImGuiCond_Always);
		bool openTree = TreeNodeEx("##NodeListSlotTreeNode", ImGuiTreeNodeFlags_DrawLinesNone | ImGuiTreeNodeFlags_Leaf);

		BeginChild("NodeListSlotChild", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		bool isFocused = IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		std::vector<int> selectedNodes;
		bool deleteSelected = isFocused && (IsKeyPressed(ImGuiKey_Delete) || IsKeyPressed(ImGuiKey_Backspace));

		for (int i = 0; i < p_nodes.size(); i++)
		{
			PushID(i);

			bool isSelected = storage->GetBool(GetID("NodeListSlotSelected"), false);
			if (isSelected)
			{
				selectedNodes.push_back(i);
			}

			std::weak_ptr<Node> &nodeWeakPtr = p_nodes[i];
			if (Selectable(nodeWeakPtr.lock() ? nodeWeakPtr.lock()->GetName().c_str() : "None", &isSelected))
			{
				if (!ImGui::GetIO().KeyShift)
				{
					selectedNodes.clear();
					storage->Clear();
					storage->SetBool(expandID, isExpanded);
				}

				selectedNodes.push_back(i);
			}
			storage->SetBool(GetID("NodeListSlotSelected"), isSelected);

			// Drag-drop target for each node
			if (BeginDragDropTarget())
			{
				bool acceptDrop = true;
				if (p_acceptCallback.has_value())
				{
					const ImGuiPayload *payload = GetDragDropPayload();
					if (payload && payload->DataSize == sizeof(Node *))
					{
						Node *droppedNode = *static_cast<Node **>(payload->Data);
						acceptDrop = (p_acceptCallback.value())(droppedNode);
					}
				}

				if (acceptDrop)
				{
					if (const ImGuiPayload *payload = AcceptDragDropPayload("SCENE_NODE"))
					{
						if (payload->DataSize == sizeof(Node *))
						{
							Node *droppedNode = *static_cast<Node **>(payload->Data);
							p_nodes[i] = droppedNode->shared_from_this();
						}
					}
				}
				else
				{
					if (BeginTooltip())
					{
						TextUnformatted(p_noAcceptMsg);
						EndTooltip();
					}
				}
				EndDragDropTarget();
			}

			PopID();
		}

		// + button, - button, clear button

		BeginDisabled(p_maxNodes >= 0 && static_cast<int>(p_nodes.size()) >= p_maxNodes);
		if (Button(ICON_LC_PLUS))
		{
			// Add a new node to the list
			p_nodes.push_back(std::weak_ptr<Node>());
		}
		EndDisabled();

		SameLine();

		BeginDisabled(p_nodes.empty());
		if (Button(ICON_LC_MINUS))
		{
			// Remove all selected nodes, or the last node if none are selected
			if (!selectedNodes.empty())
			{
				deleteSelected = true;
			}
			else
			{
				PushID((int)p_nodes.size() - 1);
				storage->SetBool(GetID("NodeListSlotSelected"), false);
				PopID();
				p_nodes.pop_back();
			}
		}
		SameLine();

		if (Button(ICON_LC_TRASH))
		{
			// Clear the list
			p_nodes.clear();
			storage->Clear();
			storage->SetBool(expandID, isExpanded);
			deleteSelected = false;
		}
		EndDisabled();

		if (isFocused && IsKeyPressed(ImGuiKey_Escape))
		{
			storage->Clear();
			storage->SetBool(expandID, isExpanded);
			deleteSelected = false;
		}

		if (deleteSelected)
		{
			if (!selectedNodes.empty())
			{
				for (size_t i = 0; i < selectedNodes.size(); ++i)
				{
					PushID(i);
					storage->SetBool(GetID("NodeListSlotSelected"), false);
					PopID();
				}

				for (int i = static_cast<int>(selectedNodes.size()) - 1; i >= 0; --i)
				{
					int index = selectedNodes[i];
					if (index >= 0 && index < static_cast<int>(p_nodes.size()))
					{
						p_nodes.erase(p_nodes.begin() + index);
					}
				}
			}
		}

		EndChild();

		if (openTree)
		{
			TreePop();
		}
	}

	EndGroup();
	PopID();

	return false;
}
#endif
