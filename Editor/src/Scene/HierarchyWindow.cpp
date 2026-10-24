#include "HierarchyWindow.hpp"
#include "InteractionState.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Component.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <EditorShortcuts.hpp>
#include <memory>


using namespace Droplet::Editor;
using namespace Droplet::Scene;

HierarchyWindow::HierarchyWindow(EditorContext *p_editorContext) :
 m_editorContext(p_editorContext)
{ }

void HierarchyWindow::InitImpl()
{   
    SetName("Node Hierarchy");
    
}

void HierarchyWindow::RenderImpl()
{
    auto scene = m_editorContext->GetActiveScene();
    if (ImGui::IsWindowFocused())
    {
        EditorShortcuts::ProcessSceneViewKeyShortcuts();
    }

    if (!scene)
    {
        return;
    }

    if (ImGui::TreeNode("Scene Selection"))
    {
        DrawSceneWindow();
        ImGui::TreePop();
    }

    if (ImGui::Button("Add Node"))
    {
        scene->AddNode("New Node");
    }
    ImGui::SameLine();

    static std::vector < std::string> nodeFiles;
    static std::string selectedNode;
    const std::filesystem::path nodesDirectory = ROOT_PATH "/GameData/Prefabs/";

    if (ImGui::Button("Add Node from file"))
    {
        RefreshLoadedNodes(nodeFiles);
        ImGui::OpenPopup("Nodes");
    }

    if (ImGui::BeginPopup("Nodes"))
    {
        if (nodeFiles.empty())
        {
            ImGui::TextUnformatted("No saved nodes found.");
        }
        for (const std::string &nodeFile : nodeFiles)
        {
            if (ImGui::Selectable(nodeFile.c_str()))
            {
                selectedNode = nodeFile;
                const std::string nodeName = std::filesystem::path(selectedNode).stem().string();
                std::string filePath = nodesDirectory.string() + std::string(selectedNode);
                scene->LoadNodeFromFile(filePath);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
    ImGui::Separator();

    // Reserve a thin strip on the left for dropping nodes as roots.
    const float rootDropWidth = 6.0f;
    const float rootDropHeight = ImGui::GetContentRegionAvail().y;

    // Left-side root drop target.
    ImGui::BeginGroup();

    ImVec2 cursorPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(cursorPos + ImVec2(0.0f, ImGui::GetScrollY()));

    ImGui::Dummy(ImVec2(rootDropWidth, rootDropHeight));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("SCENE_NODE"))
        {
            Node *draggedNode = *static_cast<Node **>(payload->Data);

            if (draggedNode)
            {
                auto draggedNodeShared = draggedNode->shared_from_this();

                // Remove from the current parent, if any.
                auto oldParent = draggedNodeShared->GetParent();

                if (oldParent)
                {
                    oldParent->RemoveChild(draggedNodeShared);
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::EndGroup();

    // Put the root nodes to the right of the drop target.
    ImGui::SameLine();

    ImGui::BeginGroup();

    for (const auto &root : scene->GetRoots())
    {
        DrawNode(root);
    }

    // Create an invisible item covering the remaining content area.
    const ImVec2 contentSize = ImGui::GetContentRegionAvail();
    
    if (contentSize.x > 0.0f && contentSize.y > 0.0f)
    {
        ImGui::Dummy(contentSize);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("SCENE_NODE"))
            {
                Node *draggedNode = *static_cast<Node **>(payload->Data);

                auto oldParent = draggedNode->shared_from_this()->GetParent();
                if (oldParent)
                {
                    oldParent->RemoveChild(draggedNode->shared_from_this());
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

    ImGui::EndGroup();

    if (m_nodeToReparent && m_newParent)
    {
        auto oldParent = m_nodeToReparent->GetParent();

        if (oldParent)
        {
            oldParent->RemoveChild(m_nodeToReparent);
        }

        m_newParent->AddChild(m_nodeToReparent);
        m_nodeToReparent.reset();
        m_newParent.reset();
    }

    if (m_nodeToAddChildTo)
    {
        auto newNode = scene->AddNode("New Node");

        m_nodeToAddChildTo->AddChild(newNode);
        m_nodeToAddChildTo.reset();
    }

    if (m_nodeToRemove)
    {
        auto oldParent = m_nodeToRemove->GetParent();

        if (oldParent)
        {
            oldParent->RemoveChild(m_nodeToRemove);
        }
		scene->RemoveRoot(m_nodeToRemove);
		m_nodeToRemove.reset();
    }
}

void Droplet::Editor::HierarchyWindow::RefreshLoadedScenes(std::vector<std::string> &p_sceneFiles)
{
    
    const std::filesystem::path scenesDirectory = ROOT_PATH "/GameData/Scenes";

    p_sceneFiles.clear();

    std::error_code error;

    if (std::filesystem::exists(scenesDirectory, error) && std::filesystem::is_directory(scenesDirectory, error))
    {
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(scenesDirectory, error))
        {
            if (entry.is_regular_file())
            {
                p_sceneFiles.push_back(
                    entry.path().filename().string()
                );
            }
        }
    }

    std::sort(p_sceneFiles.begin(), p_sceneFiles.end());
}

void Droplet::Editor::HierarchyWindow::RefreshLoadedNodes(std::vector<std::string> &p_nodeFiles)
{
    const std::filesystem::path nodesDirectory = ROOT_PATH "/GameData/Prefabs";

    p_nodeFiles.clear();

    std::error_code error;

    if (std::filesystem::exists(nodesDirectory, error) && std::filesystem::is_directory(nodesDirectory, error))
    {
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(nodesDirectory, error))
        {
            if (entry.is_regular_file())
            {
                p_nodeFiles.push_back(entry.path().filename().string());
            }
        }
    }

    std::sort(p_nodeFiles.begin(), p_nodeFiles.end());
}

void Droplet::Editor::HierarchyWindow::DrawSceneWindow()
{
    static char fileName[128] = "MyScene";

    ImGui::InputText("Scene name", fileName, sizeof(fileName));
    static std::vector <std::string> sceneFiles;
    static std::string selectedScene;
    static bool hasScannedScenes = false;
    const std::filesystem::path scenesDirectory = ROOT_PATH "/GameData/Scenes/";
    if (!hasScannedScenes)
    {
        RefreshLoadedScenes(sceneFiles);
        hasScannedScenes = true;
    }

    if (ImGui::Button("Save Scene"))
    {
        m_editorContext->GetActiveScene()->SetName(fileName);
        std::string path = (scenesDirectory).string() + std::string(fileName) + ".json";
        m_editorContext->SaveScene(path);

    }
    ImGui::SameLine();

    if (ImGui::Button("New Scene"))
    {
        std::string name = "New Scene";

        m_editorContext->CreateNewScene(name);
        m_editorContext->SetActiveScene(name);
        std::strncpy(fileName, name.c_str(), sizeof(name) - 1);

        fileName[sizeof(name) - 1] = '\0';
    }

    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        RefreshLoadedScenes(sceneFiles);
        ImGui::OpenPopup("Scenes");
    }
    

    if (ImGui::BeginPopup("Scenes"))
    {
        ImGui::SeparatorText("Saved Scenes");
        
        if (sceneFiles.empty())
        {
            ImGui::TextUnformatted("No saved scenes found.");
        }
        for (const std::string &sceneFile : sceneFiles)
        {
            if (ImGui::Selectable(sceneFile.c_str()))
            {
                selectedScene = sceneFile;

                const std::string sceneName =
                    std::filesystem::path(selectedScene)
                    .stem().string();

                std::string filePath = scenesDirectory.string() + std::string(selectedScene);

                m_editorContext->LoadFromFile(filePath);
                m_editorContext->SetActiveScene(sceneName);
                std::strncpy(fileName, sceneName.c_str(), sizeof(fileName) - 1);
                fileName[sizeof(fileName) - 1] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}


void HierarchyWindow::DrawNode(const std::shared_ptr<Node> &p_node) // TODO: Make weak pointer
{
    if (!p_node)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (p_node->GetChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (m_editorContext->GetInteractionState().IsNodeSelected(p_node))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool opened = ImGui::TreeNodeEx(p_node.get(), flags, "%s", p_node->GetName().c_str());

    const bool visible = ImGui::IsItemVisible();

	// Only handle interactions if the item is visible to avoid unnecessary processing for off-screen items
    if (visible)
    {
        if (ImGui::IsItemClicked())
        {
            m_editorContext->GetInteractionState().ClearNodeSelection();
            m_editorContext->GetInteractionState().SelectNode(p_node);
        }

        if (ImGui::BeginDragDropSource())
        {
            Node *draggedNode = p_node.get();

            ImGui::SetDragDropPayload("SCENE_NODE", &draggedNode, sizeof(Node *));

            ImGui::Text("%s", p_node->GetName().c_str());

            ImGui::EndDragDropSource();
        }


 
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Add Child"))
            {
                m_nodeToAddChildTo = p_node;
            }

            if (ImGui::MenuItem("Remove Node"))
            {
                m_nodeToRemove = p_node;
            }

            if (ImGui::MenuItem("Serialize Node"))
            {
                const std::filesystem::path prefabDirectory = ROOT_PATH "/GameData/Prefabs";
                const std::string filePath = (prefabDirectory / (p_node->GetName() + ".json")).string();
                m_editorContext->SaveNode(filePath, *p_node.get());
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("SCENE_NODE"))
            {
                Node *draggedNode =
                    *static_cast<Node **>(payload->Data);

                auto draggedNodeShared = draggedNode->shared_from_this();

                if (draggedNodeShared != p_node &&
                    !p_node->IsDescendantOf(draggedNodeShared))
                {
                    m_nodeToReparent = draggedNodeShared;
                    m_newParent = p_node;
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    if (opened)
    {
        for (const auto &child : p_node->GetChildren())
        {
            DrawNode(child);
        }
        ImGui::TreePop();
    }
}