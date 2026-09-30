#include "ResourceBrowser.hpp"

using namespace Droplet::Editor::Resource;

void ResourceBrowser::InitImpl()
{
	SetName("Resource Browser");
}

void ResourceBrowser::RenderImpl()
{
	using namespace ImGui;

	Droplet::ResourceManager *resourceManager = nullptr;

	if (!resourceManager)
	{
		return;
	}

	std::vector<const MetaEntry *> resources = resourceManager->GetRegisteredResources();

	// Split the window into two child windows: one for the resource list and one for the resource details

	ImVec2 windowSize = GetWindowSize();
	ImVec2 cursorStartPos = GetCursorPos();
	ImVec2 listSize = ImVec2(windowSize.x * 0.5f, 0);

	// Selectable and draggable resource entries
	BeginChild("Resource List", listSize, ImGuiChildFlags_ResizeX);
	{
		for (std::size_t i = 0; i < resources.size(); i++)
		{
			const MetaEntry *entry = resources[i];

			if (Selectable(entry->name.c_str(), selectedResourceGUID == entry->guid))
			{
				if (selectedResourceGUID == entry->guid)
				{
					selectedResourceGUID = C_INVALID_GUID; // Deselect the resource if it was already selected
				}
				else
				{
					selectedResourceGUID = entry->guid; // Select the resource if it was not already selected
				}
			}
		}
	}
	EndChild();

	SetCursorPos(ImVec2(cursorStartPos.x + GetItemRectSize().x, cursorStartPos.y));

	// Details of selected resource
	BeginChild("Resource Inspector");
	{
		if (selectedResourceGUID != C_INVALID_GUID)
		{
			// TODO: Fetch the resource details using the GUID and render them here
		}
	}
	EndChild();
}
