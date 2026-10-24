#pragma once
#include <string>
#include <memory>
#include <Editor/DragDropFields.hpp>
#include <SceneSystem/Node.hpp>

namespace Droplet::Editor
{
	/// @brief Creates a UI slot for dropping a Node into a field.
	/// @param p_label The label for the slot.
	/// @param p_current A weak pointer to the current Node in the slot.
	/// @param p_acceptFunc An optional function to determine if a Node is acceptable for the slot.
	/// @param p_size The size of the slot.
	/// @return true if the slot accepted a new Node, otherwise false.
	bool NodeSlot(
		const char *p_label,
		std::weak_ptr<Droplet::Scene::Node> &p_current,
		const std::function<bool(std::shared_ptr<Droplet::Scene::Node>)> &p_acceptFunc = nullptr,
		ImVec2 p_size = ImVec2(0, 0)
	);
}