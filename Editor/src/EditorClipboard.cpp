#include "EditorClipboard.hpp"

void Droplet::Editor::EditorClipboard::CopyNodes(const std::vector<nlohmann::json> &p_nodes)
{
	nlohmann::json clipboardData;

	clipboardData["format"] = "DropletNodes";
	clipboardData["nodes"] = p_nodes;

	const std::string clipboardText = clipboardData.dump(2);

	ImGui::SetClipboardText(clipboardText.c_str());


}

std::optional<std::vector<nlohmann::json>> Droplet::Editor::EditorClipboard::PasteNodes()
{
	// HACK this assumes that we only will call this function and only this function when doing ctrl+v 
	// HACK Maybe try catch should be implemented to fix this, but i heard we should avoid that
	// HACK Gupta made this... blame him in the future for being lazy and asssuming stuff
	const char *clipboardText = ImGui::GetClipboardText();


	if (clipboardText == nullptr || clipboardText[0] == '\0')
	{
		return std::nullopt;
	}

	const nlohmann::json clipboardData = nlohmann::json::parse(clipboardText);

	if (!clipboardData.contains("format") || !clipboardData["format"].is_string() || clipboardData["format"] != "DropletNodes")
	{
		return std::nullopt;
	}

	if (!clipboardData.contains("nodes") || !clipboardData["nodes"].is_array())
	{
		return std::nullopt;
	}

	std::vector<nlohmann::json> nodes;

	for (const nlohmann::json &nodeJson : clipboardData["nodes"])
	{
		if (!nodeJson.is_object())
		{
			return std::nullopt;
		}

		nodes.push_back(nodeJson);
	}

	return nodes;



}
