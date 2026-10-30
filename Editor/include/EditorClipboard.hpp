#pragma once

#include <json/json.hpp>

namespace Droplet::Editor
{
	/// @brief A helper class that is here for everything that has to do with the clipboard
	/// aka everything you copy and paste is related to this class
	class EditorClipboard
	{
	public:
		/// @brief Copies a vector of json nodes to the clipboard to be pasted later
		/// @param p_nodes The node(s) that are going to the clipboard
		static void CopyNodes(const std::vector<nlohmann::json> &p_nodes);

		/// @brief Returns a list of nodes that was in the clipboard
		/// @return 
		static std::optional<std::vector<nlohmann::json>> PasteNodes();
	};
}