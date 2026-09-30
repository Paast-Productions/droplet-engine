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

	BeginChild("Resource List", listSize, ImGuiChildFlags_ResizeX);
	{
		for (std::size_t i = 0; i < resources.size(); i++)
		{
			// TODO: Render selectable and draggable resource entry
		}
	}
	EndChild();

	SetCursorPos(ImVec2(cursorStartPos.x + GetItemRectSize().x, cursorStartPos.y));

	BeginChild("Resource Inspector");
	{
		// TODO: Render resource details when a resource is selected
	}
	EndChild();

}