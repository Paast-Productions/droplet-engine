#pragma once

#include <vector>

namespace Droplet
{
	struct MetaEntry;
}

namespace Droplet::Scene
{
	class Node;
}

namespace Droplet::Editor
{
	/// @brief Resource metadata carried by a resource drag-and-drop payload.
	using ResourceDropEntries = std::vector<const MetaEntry *>;

	/// @brief Reads resource payloads from the current ImGui drop target.
	///
	/// Must be called between BeginDragDropTarget() and EndDragDropTarget().
	/// The entries are returned while hovering. p_outIsDelivery is only true on release.
	///
	/// @param p_outIsDelivery Set to true when the payload is delivered this frame.
	/// @return The dragged resources, or an empty list if no resource is being dragged.
	ResourceDropEntries AcceptResourceDrop(bool &p_outIsDelivery);

	/// @brief Checks whether any of the entries can be applied to the node.
	/// @param p_node The node that is hovered.
	/// @param p_entries The dragged resources.
	/// @return True if at least one component of the node accepts at least one entry.
	bool CanApplyResources(const Droplet::Scene::Node &p_node, const ResourceDropEntries &p_entries);

	/// @brief Offers every entry to the node. Only call this on delivery so resources are never applied while dragging.
	/// @param p_node The node that the resources were released on.
	/// @param p_entries The released resources.
	void ApplyResources(Droplet::Scene::Node &p_node, const ResourceDropEntries &p_entries);

	/// @brief Draws the node's bounds through DebugDraw to show the drop target.
	/// @param p_node The hovered node.
	/// @param p_accepts Green when true, red when false.
	void DrawResourceDropHighlight(Droplet::Scene::Node &p_node, bool p_accepts);
}
