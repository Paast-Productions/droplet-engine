#include "ScriptWindow.hpp"

#include "ScriptSystem/ScriptSystem.hpp"
#include <string>

using namespace Droplet::Editor;

ScriptWindow::ScriptWindow(std::shared_ptr<InteractionState> p_interactionState)
{
	m_interactionState = p_interactionState;
	m_scriptPath = Droplet::Script::ScriptSystem::Get().GetScriptPath();

	RefreshScripts();
}

void ScriptWindow::InitImpl()
{
	SetName("Scripts");
}

void ScriptWindow::CloseImpl()
{
	// Cleanup for the node inspector window
}

void ScriptWindow::RenderImpl()
{
	if (m_scriptPath.empty())
	{
		ImGui::TextUnformatted("Script folder not configured");
	}

	if (!m_refreshError.empty())
	{
		ImGui::TextWrapped("Could not read scripts: %s", m_refreshError.c_str());
		return;
	}

	if (m_scriptTree.empty())
	{
		ImGui::TextDisabled("No Lua scripts found in this folder.");
		return;
	}

	for (const ScriptTreeNode& node : m_scriptTree)
	{
		RenderTreeHelper(node);
	}
}

void ScriptWindow::RenderToolbar()
{
	if (ImGui::Button("Refresh"))
	{
		RefreshScripts();
	}
	ImGui::Separator();
}

void Droplet::Editor::ScriptWindow::RenderTreeHelper(const ScriptTreeNode& p_node)
{
	const std::string id = p_node.relativePath.generic_string();
	ImGui::PushID(id.c_str());

	if (p_node.isDirectory)
	{
		if (ImGui::TreeNodeEx(p_node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (const ScriptTreeNode& child : p_node.children)
			{
				RenderTreeHelper(child);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		const std::string path = p_node.relativePath.generic_string();
		if (ImGui::Selectable(p_node.name.c_str(), m_selectedScript == path))
		{
			m_selectedScript = path;
		}
	}
	ImGui::PopID();
}

void ScriptWindow::RefreshScripts()
{
	m_scriptTree.clear();
	m_selectedScript.clear();
	m_refreshError.clear();
	m_scriptPath = Droplet::Script::ScriptSystem::Get().GetScriptPath();

	if (m_scriptPath.empty())
	{
		m_refreshError = "The script directory has not been configured.";
		return;
	}

	std::error_code errorCode;
	if (!std::filesystem::is_directory(m_scriptPath, errorCode))
	{
		m_refreshError = errorCode ? errorCode.message() : "The configured path is not a directory.";
		return;
	}

	std::filesystem::recursive_directory_iterator iterator(m_scriptPath, 
		std::filesystem::directory_options::skip_permission_denied, errorCode);

	const std::filesystem::recursive_directory_iterator end;
	if (errorCode)
	{
		m_refreshError = errorCode.message();
		return;
	}

	for (; iterator != end; iterator.increment(errorCode))
	{
		if (errorCode)
		{
			m_refreshError = errorCode.message();
			break;
		}

		if (!iterator->is_regular_file(errorCode) 
			|| iterator->path().extension() != ".lua" 
			|| iterator->path().filename().string() == "LuaHelper.d.lua")
		{
			continue;
		}

		const std::filesystem::path relativePath = iterator->path().lexically_relative(m_scriptPath);
		std::filesystem::path accumulatedPath;
		std::vector<ScriptTreeNode>* children = &m_scriptTree;

		for (auto component = relativePath.begin(); component != relativePath.end(); ++component)
		{
			const bool isFile = std::next(component) == relativePath.end();
			const std::string name = component->string();
			accumulatedPath /= *component;

			auto existing = std::find_if(children->begin(), children->end(),
				[&name](const ScriptTreeNode& p_node) { return p_node.name == name; });

			if (existing == children->end())
			{
				children->push_back({ name, accumulatedPath, !isFile, {} });
				existing = std::prev(children->end());
			}

			if (!isFile)
			{
				children = &existing->children;
			}
		}
	}

	const auto sortNodes = [](std::vector<ScriptTreeNode>& p_nodes, const auto& p_self) -> void
	{
		std::sort(p_nodes.begin(), p_nodes.end(), [](const ScriptTreeNode& p_left, const ScriptTreeNode& p_right)
			{
				if (p_left.isDirectory != p_right.isDirectory)
				{
					return p_left.isDirectory;
				}
				return p_left.name < p_right.name;
			});

		for (ScriptTreeNode& node : p_nodes)
		{
			p_self(node.children, p_self);
		}
	};
	sortNodes(m_scriptTree, sortNodes);
}
