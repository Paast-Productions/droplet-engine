#include <algorithm>
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

namespace
{
    /// @brief Gets the nodes a drag of p_dragged should move. If the dragged node is part of a multi-selection, all
    /// selected nodes are moved. Nodes whose ancestor is also moved are excluded, since they follow their ancestor.
    std::vector<std::shared_ptr<Node>> GetDraggedNodes(const InteractionState &p_state, const std::shared_ptr<Node> &p_dragged)
    {
        std::vector<std::shared_ptr<Node>> candidates;

        if (p_state.IsNodeSelected(p_dragged))
        {
            candidates = p_state.GetSelectedNodes();
        }
        else
        {
            candidates.push_back(p_dragged);
        }

        std::vector<std::shared_ptr<Node>> result;

        for (const auto &node : candidates)
        {
            bool ancestorMoved = false;

            for (auto parent = node->GetParent(); parent; parent = parent->GetParent())
            {
                if (std::find(candidates.begin(), candidates.end(), parent) != candidates.end())
                {
                    ancestorMoved = true;
                    break;
                }
            }

            if (!ancestorMoved)
            {
                result.push_back(node);
            }
        }

        return result;
    }
}

void HierarchyWindow::InitImpl()
{   
    SetName("Node Hierarchy");
    
}

void HierarchyWindow::RenderImpl()
{
    auto scene = m_editorContext->GetActiveScene();
    if (ImGui::IsWindowFocused())
    {
        EditorShortcuts::GetInstance().ProcessSceneViewKeyShortcuts(*m_editorContext);
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
    glm::vec3 cameraPos = m_editorContext->GetEditorCamera().GetPosition() + m_editorContext->GetEditorCamera().GetForward();

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

    ImGui::SeparatorText(scene->GetName().c_str());

    // Reserve a thin strip on the left for dropping nodes as roots.
    const float rootDropWidth = 6.0f;
    const float rootDropHeight = ImGui::GetContentRegionAvail().y;

    // Left-side root drop target.
    ImGui::BeginGroup();

    ImVec2 cursorPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(cursorPos + ImVec2(0.0f, ImGui::GetScrollY()));

    ImGui::Dummy(ImVec2(rootDropWidth, rootDropHeight));

    // The node area starts at the top of the root drop strip and spans the rest of the window
    const ImVec2 nodeAreaMin = ImGui::GetItemRectMin();
    const ImVec2 nodeAreaMax = ImGui::GetWindowPos() + ImGui::GetWindowSize();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("SCENE_NODE"))
        {
            Node *draggedNode = *static_cast<Node **>(payload->Data);

            if (draggedNode)
            {
                // Remove all dragged nodes from their current parents, if any.
                for (const auto &node : GetDraggedNodes(m_editorContext->GetInteractionState(), draggedNode->shared_from_this()))
                {
                    auto oldParent = node->GetParent();

                    if (oldParent)
                    {
                        oldParent->RemoveChild(node);
                    }
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PREFAB_PATH"))
        {
            if (payload->Data != nullptr && payload->DataSize > 0)
            {
                const char *droppedData = static_cast<const char *>(payload->Data);
                glm::vec3 position = m_editorContext->GetEditorCamera().GetPosition() + m_editorContext->GetEditorCamera().GetForward() * 5.0f;

                scene->LoadNodeFromFile(droppedData,position);
            }
        }
        ImGui::EndDragDropTarget();

    }
    ImGui::EndGroup();

    // Put the root nodes to the right of the drop target.
    ImGui::SameLine();

    ImGui::BeginGroup();

    m_visibleNodes.clear();
    m_visibleNodeRects.clear();
    m_rangeSelectTarget = nullptr;

    for (const auto &root : scene->GetRoots())
    {
        DrawNode(root);
    }

    // Resolve a shift-click range selection now that the display order of all nodes is known
    if (m_rangeSelectTarget)
    {
        InteractionState &interactionState = m_editorContext->GetInteractionState();
        const std::shared_ptr<Node> anchor = m_selectionAnchor.lock();

        const auto targetIt = std::find(m_visibleNodes.begin(), m_visibleNodes.end(), m_rangeSelectTarget);
        auto anchorIt = anchor ? std::find(m_visibleNodes.begin(), m_visibleNodes.end(), anchor) : m_visibleNodes.end();

        if (anchorIt == m_visibleNodes.end())
        {
            // No usable anchor, so the target becomes the anchor
            anchorIt = targetIt;
            m_selectionAnchor = m_rangeSelectTarget;
        }

        if (targetIt != m_visibleNodes.end())
        {
            if (!m_rangeSelectAdditive)
            {
                interactionState.ClearNodeSelection();
            }

            for (auto it = std::min(anchorIt, targetIt); it <= std::max(anchorIt, targetIt); ++it)
            {
                interactionState.SelectNode(*it, false);
            }
        }

        m_rangeSelectTarget = nullptr;
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

                for (const auto &node : GetDraggedNodes(m_editorContext->GetInteractionState(), draggedNode->shared_from_this()))
                {
                    auto oldParent = node->GetParent();

                    if (oldParent)
                    {
                        oldParent->RemoveChild(node);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PREFAB_PATH"))
        {
            if (payload->Data != nullptr && payload->DataSize > 0)
            {
                const char *droppedData = static_cast<const char *>(payload->Data);
                glm::vec3 position = m_editorContext->GetEditorCamera().GetPosition() + m_editorContext->GetEditorCamera().GetForward()*5.0f;
                scene->LoadNodeFromFile(droppedData, position);
            }
        }
        ImGui::EndDragDropTarget();

    }

    ImGui::EndGroup();

    HandleRectSelection(nodeAreaMin, nodeAreaMax);

    if (!m_nodesToReparent.empty() && m_newParent)
    {
        for (const auto &node : m_nodesToReparent)
        {
            // Skip invalid moves, such as moving a node into itself or its own descendants
            if (node == m_newParent || m_newParent->IsDescendantOf(node))
            {
                continue;
            }

            auto oldParent = node->GetParent();

            if (oldParent)
            {
                oldParent->RemoveChild(node);
            }

            m_newParent->AddChild(node);
        }
    }

    m_nodesToReparent.clear();
    m_newParent.reset();

    if (m_nodeToAddChildTo)
    {
        auto newNode = scene->AddNode("New Node");

        m_nodeToAddChildTo->AddChild(newNode);
        m_nodeToAddChildTo.reset();
    }

    if (!m_nodesToRemove.empty())
    {
        for (const auto &node : m_nodesToRemove)
        {
            auto oldParent = node->GetParent();

            if (oldParent)
            {
                oldParent->RemoveChild(node);
            }

            scene->RemoveRoot(node);
        }
        m_nodesToRemove.clear();
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
        m_editorContext->GetActiveScene().get()->SetActive(false);
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

                m_editorContext->GetActiveScene().get()->SetActive(false);
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

    ImGuiTreeNodeFlags flags = 
        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | 
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

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

    m_visibleNodes.push_back(p_node);
    m_visibleNodeRects.emplace_back(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

	// Only handle interactions if the item is visible to avoid unnecessary processing for off-screen items
    if (visible)
    {
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
        {
            InteractionState &interactionState = m_editorContext->GetInteractionState();
            const ImGuiIO &io = ImGui::GetIO();

            if (io.KeyShift)
            {
                // Select the range between the anchor and this node
                m_rangeSelectTarget = p_node;
                m_rangeSelectAdditive = io.KeyCtrl;
            }
            else if (io.KeyCtrl)
            {
                // Toggle this node, keeping the rest of the selection
                if (interactionState.IsNodeSelected(p_node))
                {
                    interactionState.DeselectNode(p_node);
                }
                else
                {
                    interactionState.SelectNode(p_node, false);
                }

                m_selectionAnchor = p_node;
            }
            else if (interactionState.IsNodeSelected(p_node) && interactionState.GetSelectedNodes().size() > 1)
            {
                // Keep the multi-selection so it can be dragged. It collapses to this node on release if no drag happened.
                m_pendingCollapseNode = p_node;
            }
            else
            {
                interactionState.ClearNodeSelection();
                interactionState.SelectNode(p_node);
                m_selectionAnchor = p_node;
            }
        }

        if (m_pendingCollapseNode == p_node && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            const ImGuiIO &io = ImGui::GetIO();
            constexpr float c_dragThreshold = 4.0f;

            if (ImGui::IsItemHovered() && io.MouseDragMaxDistanceSqr[ImGuiMouseButton_Left] < c_dragThreshold * c_dragThreshold)
            {
                InteractionState &interactionState = m_editorContext->GetInteractionState();
                interactionState.ClearNodeSelection();
                interactionState.SelectNode(p_node);
                m_selectionAnchor = p_node;
            }

            m_pendingCollapseNode = nullptr;
        }

        if (ImGui::BeginDragDropSource())
        {
            Node *draggedNode = p_node.get();

            ImGui::SetDragDropPayload("SCENE_NODE", &draggedNode, sizeof(Node *));

            const size_t draggedCount = GetDraggedNodes(m_editorContext->GetInteractionState(), p_node).size();

            if (draggedCount > 1)
            {
                ImGui::Text("%s (+%zu more)", p_node->GetName().c_str(), draggedCount - 1);
            }
            else
            {
                ImGui::Text("%s", p_node->GetName().c_str());
            }

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
				// If selected, remove all selected nodes. Otherwise, remove this node only.
                if (m_editorContext->GetInteractionState().IsNodeSelected(p_node))
                {
                    for (const auto &selectedNode : m_editorContext->GetInteractionState().GetSelectedNodes())
                    {
                        m_nodesToRemove.push_back(selectedNode);
                    }
                }
                else
                {
                    m_nodesToRemove.push_back(p_node);
				}
            }

            if (ImGui::MenuItem("Serialize Node"))
            {
                const std::filesystem::path prefabDirectory = ROOT_PATH "/GameData/Prefabs";
                const std::string filePath = (prefabDirectory / (p_node->GetName() + ".json")).string();
                m_editorContext->SaveNode(filePath, *p_node.get());
            }

            if (ImGui::MenuItem("Duplicate Node"))
            {
				// TODO: Implement node duplication logic
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
                    m_nodesToReparent = GetDraggedNodes(m_editorContext->GetInteractionState(), draggedNodeShared);
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

bool HierarchyWindow::IsOverNodeRow(const ImVec2 &p_pos) const
{
    for (const auto &[rowMin, rowMax] : m_visibleNodeRects)
    {
        if (p_pos.x >= rowMin.x && p_pos.x <= rowMax.x && p_pos.y >= rowMin.y && p_pos.y <= rowMax.y)
        {
            return true;
        }
    }

    return false;
}

void HierarchyWindow::HandleRectSelection(const ImVec2 &p_emptyAreaMin, const ImVec2 &p_emptyAreaMax)
{
    InteractionState &interactionState = m_editorContext->GetInteractionState();
    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

    // Start the drag when pressing in the empty area below the nodes
    if (!m_marqueeActive)
    {
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGui::IsAnyItemActive() && ImGui::IsMouseHoveringRect(p_emptyAreaMin, p_emptyAreaMax)
            && !IsOverNodeRow(io.MousePos))
        {
            m_marqueeActive = true;
            m_marqueeStart = io.MousePos + scroll;
        }

        return;
    }

    const ImVec2 start = m_marqueeStart - scroll;
    const ImVec2 end = io.MousePos;
    const ImVec2 rectMin = ImVec2(std::min(start.x, end.x), std::min(start.y, end.y));
    const ImVec2 rectMax = ImVec2(std::max(start.x, end.x), std::max(start.y, end.y));

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(60, 120, 220, 50));
        drawList->AddRect(rectMin, rectMax, IM_COL32(60, 120, 220, 200));
        return;
    }

    // Released: select all rows touched by the rectangle
    m_marqueeActive = false;

    const bool additive = io.KeyShift || io.KeyCtrl;
    constexpr float c_minDragSize = 4.0f;
    const bool isRect = (rectMax.x - rectMin.x) >= c_minDragSize || (rectMax.y - rectMin.y) >= c_minDragSize;

    if (!additive)
    {
        interactionState.ClearNodeSelection();
    }

    if (!isRect)
    {
        return;
    }

    for (size_t i = 0; i < m_visibleNodes.size() && i < m_visibleNodeRects.size(); i++)
    {
        const auto &[rowMin, rowMax] = m_visibleNodeRects[i];

        const bool overlaps = rowMin.x <= rectMax.x && rowMax.x >= rectMin.x
            && rowMin.y <= rectMax.y && rowMax.y >= rectMin.y;

        if (!overlaps)
        {
            continue;
        }

        if (io.KeyCtrl && interactionState.IsNodeSelected(m_visibleNodes[i]))
        {
            interactionState.DeselectNode(m_visibleNodes[i]);
        }
        else
        {
            interactionState.SelectNode(m_visibleNodes[i], false);
        }
    }
}
