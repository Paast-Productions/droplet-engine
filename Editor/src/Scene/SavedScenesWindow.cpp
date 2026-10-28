#include "SavedScenesWindow.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#undef WIN32_LEAN_AND_MEAN
#endif


Droplet::Editor::SavedScenesWindow::SavedScenesWindow()
{
    m_scenePath = ROOT_PATH "/GameData/Scenes/";
    RefreshScannedFiles(m_sceneFiles);
}

void Droplet::Editor::SavedScenesWindow::InitImpl()
{
    SetName("Saved Scenes");
    ToggleToolbar(true);
}

void Droplet::Editor::SavedScenesWindow::RenderImpl()
{
    if (m_sceneTree.empty())
    {
        ImGui::TextDisabled("No scenes found in this folder");
        return;
    }

    for (const SceneTreeNode &node : m_sceneTree)
    {
        RenderTreeHelper(node);
    }
}

void Droplet::Editor::SavedScenesWindow::CloseImpl()
{
}

void Droplet::Editor::SavedScenesWindow::RenderToolbar()
{
    if (ImGui::MenuItem("Refresh"))
    {
        RefreshScannedFiles(m_sceneFiles);
    }
    ImGui::Separator();
    
}

void Droplet::Editor::SavedScenesWindow::RenderTreeHelper(const SceneTreeNode &p_node)
{
    const std::string id = p_node.relativePath.generic_string();
    ImGui::PushID(id.c_str());

    if (p_node.isDirectory)
    {
        if (ImGui::TreeNodeEx(p_node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            for (const SceneTreeNode &child : p_node.children)
            {
                RenderTreeHelper(child);
            }
            ImGui::TreePop();
        }
    }
    else
    {
        const std::string path = p_node.relativePath.generic_string();
        const bool isSelected = m_selectedJsonScene == path;

        if (ImGui::Selectable(p_node.name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
        {
            m_selectedJsonScene = path;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                OpenSceneJson(p_node.relativePath);
            }
        }

        if (ImGui::BeginPopupContextItem("SceneContextMenu"))
        {

            if (ImGui::MenuItem("Delete"))
            {
                m_selectedJsonScene = path;
                m_openDelete = true;
            }

            ImGui::EndPopup();
        }

        if (m_openDelete)
        {
            ImGui::OpenPopup("DeleteSceneConfirm");
            m_openDelete = false;
        }

        if (ImGui::BeginPopupModal("DeleteSceneConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you SURE you want to DELETE the scene %s\n", m_selectedJsonScene.c_str());

            const float buttonWidth = 80.0f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float totalWidth = buttonWidth * 2.0f + spacing;

            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);
            
            if (ImGui::Button("Delete", ImVec2(buttonWidth, 0.0f)))
            {
                DeleteSelectedScene();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::PopID();
}

void Droplet::Editor::SavedScenesWindow::DeleteSelectedScene()
{
}

void Droplet::Editor::SavedScenesWindow::OpenSceneJson(const std::filesystem::path &p_path)
{
#ifdef _WIN32
    std::error_code errorCode;
    const std::filesystem::path absolutePath = std::filesystem::absolute(m_scenePath / p_path, errorCode);
    if (errorCode)
    {
        return;
    }
    const HINSTANCE result = ShellExecuteW(nullptr, L"open", absolutePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    const std::intptr_t resultCode = reinterpret_cast<std::intptr_t>(result);
    if (resultCode <= 32)
    {
        //Could not open file with default application
    }

#else
    //can only open on windows
#endif
}

void Droplet::Editor::SavedScenesWindow::RenameScene()
{
}

void Droplet::Editor::SavedScenesWindow::RefreshScannedFiles(std::vector<std::string> &p_sceneFiles)
{
    m_sceneTree.clear();
    m_selectedJsonScene.clear();

    std::error_code errorCode;
    if (!std::filesystem::is_directory(m_scenePath, errorCode))
    {
        return;
    }

    std::filesystem::recursive_directory_iterator iterator(m_scenePath, std::filesystem::directory_options::skip_permission_denied, errorCode);

    const std::filesystem::recursive_directory_iterator end;
    if (errorCode)
    {
        return;
    }

    for (; iterator != end; iterator.increment(errorCode))
    {
        if (errorCode)
        {
            break;
        }

        if (!iterator->is_regular_file(errorCode) || iterator->path().extension() != ".json")
        {
            continue;
        }

        const std::filesystem::path relativePath = iterator->path().lexically_relative(m_scenePath);
        std::filesystem::path accumulatedPath;
        std::vector<SceneTreeNode> *children = &m_sceneTree;

        for (auto obj = relativePath.begin(); obj != relativePath.end(); ++obj)
        {
            const bool isFile = std::next(obj) == relativePath.end();
            const std::string name = obj->string();
            accumulatedPath /= *obj;

            auto existing = std::find_if(children->begin(), children->end(), [&name](const SceneTreeNode &p_node) {return p_node.name == name;});

            if (existing == children->end())
            {
                children->push_back({ name,!isFile,accumulatedPath, {} });
                existing = std::prev(children->end());
            }

            if (!isFile)
            {
                children = &existing->children;
            }
        }
    }

    const auto sortNodes = [](std::vector<SceneTreeNode> &p_nodes, const auto &p_self)->void
        {
            std::sort(p_nodes.begin(), p_nodes.end(), [](const SceneTreeNode &p_left, const SceneTreeNode &p_right)
                {
                    if (p_left.isDirectory != p_right.isDirectory);
                    {
                        return p_left.isDirectory;
                    }
                    return p_left.name < p_right.name;
                });
            for (SceneTreeNode &node : p_nodes)
            {
                p_self(node.children, p_self);
            }
        };
    sortNodes(m_sceneTree, sortNodes);
}
