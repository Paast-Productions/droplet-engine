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
#endif
