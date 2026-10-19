#include "ScriptsWindow.hpp"

#include <ScriptSystem/ScriptSystem.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdint>
#include <cctype>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#endif

using namespace Droplet::Editor;

ScriptsWindow::ScriptsWindow()
{
	m_scriptPath = Droplet::Script::ScriptSystem::Get().GetScriptPath();
	RefreshScripts();
}

void ScriptsWindow::InitImpl()
{
	SetName("Scripts");
	ToggleToolbar(true);
}

void ScriptsWindow::CloseImpl()
{
	// Cleanup for the node inspector window
}

void ScriptsWindow::RenderImpl()
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

	for (const ScriptTreeNode &node : m_scriptTree)
	{
		RenderTreeHelper(node);
	}

}

void ScriptsWindow::RenderToolbar()
{
	if (ImGui::Button("Refresh"))
	{
		RefreshScripts();
	}

	if (ImGui::Button("Add+"))
	{
		ImGui::OpenPopup("Create Lua Script");
	}

	RenderAddScript();
	ImGui::Separator();
}

void ScriptsWindow::RenderTreeHelper(const ScriptTreeNode &p_node)
{
	const std::string id = p_node.relativePath.generic_string();
	ImGui::PushID(id.c_str());

	if (p_node.isDirectory)
	{
		if (ImGui::TreeNodeEx(p_node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (const ScriptTreeNode &child : p_node.children)
			{
				RenderTreeHelper(child);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		const std::string path = p_node.relativePath.generic_string();
		const bool isSelected = m_selectedScript == path;

		if (ImGui::Selectable(p_node.name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
		{
			m_selectedScript = path;
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				OpenScript(p_node.relativePath);
			}
		}

		if (ImGui::BeginPopupContextItem("ScriptContextMenu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				m_selectedScript = path;
				m_openDeleteConfirmation = true;
			}
			ImGui::EndPopup();
		}

		if (m_openDeleteConfirmation)
		{
			ImGui::OpenPopup("DeleteScriptConfirm");
			m_openDeleteConfirmation = false;
		} 

		if (ImGui::BeginPopupModal("DeleteScriptConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure you want to delete the script %s?\n", m_selectedScript.c_str());

			const float buttonWidth = 80.0f;
			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float totalWidth = buttonWidth * 2.0f + spacing;

			ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);

			if (ImGui::Button("Delete", ImVec2(buttonWidth, 0.0f)))
			{
				DeleteSelectedScript();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("SCRIPT_PATH", path.c_str(), path.size() + 1);
			ImGui::TextUnformatted(p_node.name.c_str());
			ImGui::EndDragDropSource();
		}
	}
	ImGui::PopID();
}

void ScriptsWindow::RefreshScripts()
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
		std::vector<ScriptTreeNode> *children = &m_scriptTree;

		for (auto component = relativePath.begin(); component != relativePath.end(); ++component)
		{
			const bool isFile = std::next(component) == relativePath.end();
			const std::string name = component->string();
			accumulatedPath /= *component;

			auto existing = std::find_if(children->begin(), children->end(),
				[&name](const ScriptTreeNode &p_node) { return p_node.name == name; });

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

	const auto sortNodes = [](std::vector<ScriptTreeNode> &p_nodes, const auto &p_self) -> void
		{
			std::sort(p_nodes.begin(), p_nodes.end(), [](const ScriptTreeNode &p_left, const ScriptTreeNode &p_right)
				{
					if (p_left.isDirectory != p_right.isDirectory)
					{
						return p_left.isDirectory;
					}
					return p_left.name < p_right.name;
				});

			for (ScriptTreeNode &node : p_nodes)
			{
				p_self(node.children, p_self);
			}
		};
	sortNodes(m_scriptTree, sortNodes);
}

void ScriptsWindow::RenderAddScript()
{
	if (ImGui::BeginPopupModal("Create Lua Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char filename[128] = "";
		static std::string errorMessage;

		const std::string scriptTemplate = R"lua(
function OnStart()
	-- Called when script starts
end

function OnUpdate(dt)
	-- Called every frame
end
)lua";

		ImGui::SetKeyboardFocusHere();
		const bool enterPressed = ImGui::InputText("Filename", filename, sizeof(filename), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("Create") || enterPressed)
		{
			std::string name = filename;

			if (name.empty())
			{
				errorMessage = "Enter filename";
			}
			else 
			{
				if (std::filesystem::path(name).extension() != ".lua")
				{
					name += ".lua";
				}

				if (std::filesystem::path(name).filename() != name || name == "." || name == "..")
				{
					errorMessage = "Invalid filename";
				}
				else 
				{
					const std::filesystem::path scriptPath = m_scriptPath / name;

					std::error_code error;

					if (std::filesystem::exists(scriptPath, error))
					{
						errorMessage = "A script with this name already exists";
					}
					else if (error)
					{
						errorMessage = "Failed to check wheter the file exist";
					}
					else
					{
						std::ofstream file(scriptPath);

						file << scriptTemplate;
						
						if (!file) 
						{
							errorMessage = "Failed to create the file";
						}
						else
						{
							file.close();

							filename[0] = '\0';
							errorMessage.clear();

							ImGui::CloseCurrentPopup();
							RefreshScripts();
						}
					}	
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			filename[0] = '\0';
			ImGui::CloseCurrentPopup();
		}

		if (!errorMessage.empty())
		{
			ImGui::TextWrapped("%s", errorMessage.c_str());
		}

		ImGui::EndPopup();
	}

	if (m_needsRefresh)
	{
		RefreshScripts();
		m_needsRefresh = false;
	}
}

void ScriptsWindow::DeleteSelectedScript()
{
	const std::filesystem::path scriptPath = m_scriptPath / m_selectedScript;
	std::error_code error;

	if (std::filesystem::remove(scriptPath, error))
	{
		m_selectedScript.clear();
		m_needsRefresh = true;
	}
}

void ScriptsWindow::OpenScript(const std::filesystem::path &p_relativePath)
{
	m_openError.clear();

#ifdef _WIN32
	std::error_code errorCode;
	const std::filesystem::path absolutePath = std::filesystem::absolute(m_scriptPath / p_relativePath, errorCode);

	if (errorCode)
	{
		m_openError = errorCode.message();
		return;
	}

	const HINSTANCE result = ShellExecuteW(nullptr, L"open", absolutePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

	const std::intptr_t resultCode = reinterpret_cast<std::intptr_t>(result);
	if (resultCode <= 32)
	{
		m_openError = "Windows could not open the file with its default application (error "
			+ std::to_string(resultCode) + ").";
	}
#else
	m_openError = "Opening scripts only works for Windows";
#endif
}