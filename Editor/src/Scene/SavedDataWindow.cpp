#include "SavedDataWindow.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#undef WIN32_LEAN_AND_MEAN
#endif


Droplet::Editor::SavedDataWindow::SavedDataWindow()
{
    m_scenePath = ROOT_PATH "/GameData/Scenes/";
    m_prefabPath = ROOT_PATH "/GameData/Prefabs/";
    RefreshScannedFiles(m_sceneFiles);
}

void Droplet::Editor::SavedDataWindow::InitImpl()
{
    SetName("Saved Gamedata");
    ToggleToolbar(true);
}

void Droplet::Editor::SavedDataWindow::RenderImpl()
{
    ImGui::SeparatorText("Scenes");

    if (m_sceneTree.empty())
    {
        ImGui::TextDisabled("No scenes found in this folder");
    }
    else
    {
        for (const SceneTreeNode &node : m_sceneTree)
        {
            RenderTreeHelper(node);
        }
    }


    ImGui::SeparatorText("Prefabs");

    if (m_prefabTree.empty())
    {
        ImGui::TextDisabled("No prefabs found in this folder");
        return;
    }

    for (const SceneTreeNode &node : m_prefabTree)
    {
        RenderTreeHelper(node);
    }

    if (m_needsRefresh)
    {
        m_needsRefresh = false;
        RefreshScannedFiles(m_sceneFiles);
    }
}

void Droplet::Editor::SavedDataWindow::CloseImpl()
{
}

void Droplet::Editor::SavedDataWindow::RenderToolbar()
{
    if (ImGui::MenuItem("Refresh"))
    {
        RefreshScannedFiles(m_sceneFiles);
    }
    ImGui::Separator();
    
}

void Droplet::Editor::SavedDataWindow::RenderTreeHelper(const SceneTreeNode &p_node)
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
        const std::string path = p_node.relativePath.string();
        const bool isSelected = m_selectedJsonScene == path;

        if (ImGui::Selectable(p_node.name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
        {
            m_selectedJsonScene = path;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                OpenJson(p_node.relativePath);
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
            ImGui::Text("Are you SURE you want to DELETE the file %s\n", m_selectedJsonScene.c_str());

            const float buttonWidth = 80.0f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float totalWidth = buttonWidth * 2.0f + spacing;

            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);
            
            if (ImGui::Button("Delete", ImVec2(buttonWidth, 0.0f)))
            {
                DeleteSelectedFile();
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

void Droplet::Editor::SavedDataWindow::DeleteSelectedFile()
{
    if (m_selectedJsonScene.empty())
    {
        return;
    }

    std::error_code errorCode;

    const bool removed = std::filesystem::remove(
        std::filesystem::path(m_selectedJsonScene),
        errorCode);

    if (errorCode)
    {
        // TODO Report to the logger
        return;
    }

    if (removed)
    {
        m_needsRefresh = true;
    }
}


void Droplet::Editor::SavedDataWindow::OpenJson(const std::filesystem::path &p_path)
{
#ifdef _WIN32
    std::error_code errorCode;

    const std::filesystem::path absolutePath =
        std::filesystem::absolute(p_path, errorCode);

    if (errorCode)
    {
        return;
    }

    const HINSTANCE result = ShellExecuteW(
        nullptr,
        L"open",
        absolutePath.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL);

    const std::intptr_t resultCode =
        reinterpret_cast<std::intptr_t>(result);

    if (resultCode <= 32)
    {
        // TODO: Report that the file could not be opened.
    }
#endif
}

void Droplet::Editor::SavedDataWindow::RefreshScannedFiles(std::vector<std::string> &p_sceneFiles)
{
    m_sceneTree.clear();
    m_prefabTree.clear();
    m_selectedJsonScene.clear();
    p_sceneFiles.clear();


    const auto scanDirectory = [&p_sceneFiles](const std::filesystem::path &p_rootPath, std::vector<SceneTreeNode> &p_tree)
        {
            std::error_code errorCode;
            if (!std::filesystem::is_directory(p_rootPath, errorCode))
            {
                return;
            }

            std::filesystem::recursive_directory_iterator iterator(p_rootPath, std::filesystem::directory_options::skip_permission_denied, errorCode);

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

                const std::filesystem::path relativePath = iterator->path().lexically_relative(p_rootPath);
                std::filesystem::path accumulatedPath = p_rootPath;
                std::vector<SceneTreeNode> *children = &p_tree;

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

                p_sceneFiles.push_back(iterator->path().string());
            }
        };

    scanDirectory(m_scenePath, m_sceneTree);
    scanDirectory(m_prefabPath, m_prefabTree);




    const auto sortNodes = [](std::vector<SceneTreeNode> &p_nodes, const auto &p_self)->void
        {
            std::sort(p_nodes.begin(), p_nodes.end(), [](const SceneTreeNode &p_left, const SceneTreeNode &p_right)
                {
                    if (p_left.isDirectory != p_right.isDirectory)
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
    sortNodes(m_prefabTree, sortNodes);
}
