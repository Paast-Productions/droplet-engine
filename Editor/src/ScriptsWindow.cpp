#include "ScriptsWindow.hpp"
#include <ScriptSystem/ScriptSystem.hpp>

#include <string>
#include <filesystem>

using namespace Droplet::Editor;

ScriptsWindow::ScriptsWindow(std::shared_ptr<InteractionState> p_interactionState)
{
	m_interactionState = p_interactionState;
}

void ScriptsWindow::InitImpl()
{
	SetName("Scripts");
}

void ScriptsWindow::CloseImpl()
{
}

void ScriptsWindow::RenderImpl()
{
	const std::string scriptPath = Droplet::Script::ScriptSystem::Get().GetScriptPath();
	const std::filesystem::path scriptDirectory(scriptPath);

	if (!std::filesystem::exists(scriptDirectory) || !std::filesystem::is_directory(scriptDirectory))
	{
		ImGui::Text("Script directory does not exist: %s", scriptPath.c_str());
		return;
	}

	for (const auto &mapEntry : std::filesystem::directory_iterator(scriptPath))
	{
		if (!mapEntry.is_directory())
		{
			continue;
		}

		const std::string mapName = mapEntry.path().filename().string();

		if (ImGui::TreeNode(mapName.c_str()))
		{
			for (const auto &entry : std::filesystem::directory_iterator(mapEntry.path()))
			{
				if (!entry.is_regular_file() || entry.path().extension() != ".lua")
				{
					continue;
				}
				if (entry.path().filename() == "LuaHelper.d.lua")
				{
					continue;
				}
				const std::string scriptName = entry.path().filename().string();
				ImGui::Text("%s", scriptName.c_str());
			}
			ImGui::TreePop();
		}
	}
}

void ScriptsWindow::RenderToolbar()
{

}
