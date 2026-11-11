#include <algorithm>
#include "HierarchyWindow.hpp"
#include "InteractionState.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Component.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <EditorShortcuts.hpp>
#include <Editor/DragDropFields.hpp>
#include <memory>


using namespace Droplet::Editor;
using namespace Droplet::Scene;

namespace
{
    /// @brief Gets the nodes a drag of p_dragged should move. If the dragged node is part of a multi-selection, all
    /// selected nodes are moved. Nodes whose ancestor is also moved are excluded, since they follow their ancestor.
    std::vector<std::weak_ptr<Node>> GetDraggedNodes(
        const InteractionState &p_state,
        const std::weak_ptr<Node> &p_dragged)
    {
        std::vector<std::weak_ptr<Node>> candidates;

        if (p_state.IsNodeSelected(p_dragged))
        {
            candidates = p_state.GetSelectedNodes();
        }
        else
        {
            candidates.push_back(p_dragged);
        }

        std::vector<std::weak_ptr<Node>> result;

        for (const auto &weakNode : candidates)
        {
            auto node = weakNode.lock();
            if (!node)
            {
                continue;
            }

            bool ancestorMoved = false;

            for (auto parent = node->GetParent().lock(); parent; parent = parent->GetParent().lock())
            {
                const bool parentIsCandidate = std::any_of(
                    candidates.begin(),
                    candidates.end(),
                    [&parent](const std::weak_ptr<Node> &p_candidate)
                    {
                        return p_candidate.lock() == parent;
                    });

                if (parentIsCandidate)
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
    ToggleToolbar(true);
}

void HierarchyWindow::RenderToolbar()
{
    auto scene = m_editorContext->GetActiveScene().lock();

	bool isEnabled = scene && !EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InPlayMode);

    if (ImGui::BeginMenu("Scene", isEnabled))
    {
        DrawSceneWindow();

		ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Nodes", isEnabled))
    {
        std::weak_ptr<Node> newNode{};

        if (ImGui::Button("Add Empty Node"))
        {
            newNode = scene->AddNode("Empty Node");
        }

        if (ImGui::Button("Add Mesh Node"))
        {
            newNode = scene->AddNode("Mesh Node");
			newNode.lock()->AttachComponent("MeshComponent");
        }

        if (ImGui::Button("Add Camera Node"))
        {
            newNode = scene->AddNode("Camera Node");
			newNode.lock()->AttachComponent("CameraComponent");
        }

        if (ImGui::Button("Add Script Node"))
        {
            newNode = scene->AddNode("Script Node");
			newNode.lock()->AttachComponent("ScriptComponent");
        }

        if (ImGui::Button("Add Physics Node"))
        {
            newNode = scene->AddNode("Physics Node");
			newNode.lock()->AttachComponent("PhysicsComponent");
        }

        if (ImGui::Button(ICON_LC_VOLUME_2 " Add Audio Emitter Node"))
        {
            newNode = scene->AddNode("Audio Emitter Node");
            newNode.lock()->AttachComponent("AudioEmitterComponent");
        }

		if (!newNode.expired())
		{
            m_editorContext->GetInteractionState().SelectNode(newNode, true);
            m_editorContext->SetNodeScrollFocus(newNode);

			// Move the new node to the center of the view
			auto &editorCamera = m_editorContext->GetEditorCamera();
			glm::vec3 position = editorCamera.GetPosition() + editorCamera.GetForward() * EditorConfig::LoadFloat("Editor.CameraPivotRadius", 8.0f);
			newNode.lock()->GetTransform().SetPosition(position, Transform::Space::World);
		}

		ImGui::EndMenu();
    }
}

void HierarchyWindow::RenderImpl()
{
    auto scene = m_editorContext->GetActiveScene().lock();
    if (ImGui::IsWindowFocused())
    {
        EditorShortcuts::GetInstance().ProcessSceneViewKeyShortcuts(*m_editorContext);
    }

    if (!scene)
    {
        return;
    }

    ImGui::SeparatorText(scene->GetName().c_str());

    ImGui::BeginDisabled(EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InPlayMode));

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
                    auto oldParent = node.lock()->GetParent();

                    if (!oldParent.expired())
                    {
                        oldParent.lock()->RemoveChild(node.lock(), true);
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

                scene->LoadNodeFromFile(droppedData, position);
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
    m_rangeSelectTarget.reset();

    ImGui::EndDisabled();

    for (const auto &root : scene->GetRoots())
    {
        DrawNode(root);
    }

    ImGui::BeginDisabled(EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InPlayMode));

    // Resolve a shift-click range selection now that the display order of all nodes is known
    if (auto target = m_rangeSelectTarget.lock())
    {
        InteractionState &interactionState = m_editorContext->GetInteractionState();
        auto anchor = m_selectionAnchor.lock();

        const auto targetIt = std::find_if(
            m_visibleNodes.begin(),
            m_visibleNodes.end(),
            [&target](const std::weak_ptr<Node> &p_node) {
                return p_node.lock() == target;
            });

        auto anchorIt = anchor
            ? std::find_if(
                m_visibleNodes.begin(),
                m_visibleNodes.end(),
                [&anchor](const std::weak_ptr<Node> &p_node) {
                    return p_node.lock() == anchor;
                })
            : m_visibleNodes.end();

        if (anchorIt == m_visibleNodes.end())
        {
            // No usable anchor, so the target becomes the anchor
            anchorIt = targetIt;
            m_selectionAnchor = target;
        }

        if (targetIt != m_visibleNodes.end())
        {
            if (!m_rangeSelectAdditive)
            {
                interactionState.ClearNodeSelection();
            }

            for (auto it = std::min(anchorIt, targetIt);
                it <= std::max(anchorIt, targetIt); ++it)
            {
                if (auto node = it->lock())
                {
                    interactionState.SelectNode(node, false);
                }
            }
        }

        m_rangeSelectTarget.reset();
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

                if (auto draggedNodeShared = draggedNode->weak_from_this().lock())
                {
                    for (const auto &weakNode : GetDraggedNodes(
                        m_editorContext->GetInteractionState(),
                        draggedNodeShared))
                    {
                        auto node = weakNode.lock();
                        if (!node)
                        {
                            continue;
                        }

                        if (auto oldParent = node->GetParent().lock())
                        {
                            oldParent->RemoveChild(node, true);
                        }
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

    if (!m_nodesToReparent.empty() && !m_newParent.expired())
    {
        for (const auto &node : m_nodesToReparent)
        {
            // Skip invalid moves, such as moving a node into itself or its own descendants
            if (node.lock() == m_newParent.lock() || m_newParent.lock()->IsDescendantOf(node.lock()))
            {
                continue;
            }

            auto oldParent = node.lock()->GetParent();

            if (!oldParent.expired())
            {
                oldParent.lock()->RemoveChild(node.lock(), true);
            }

            m_newParent.lock()->AddChild(node.lock(), true);
        }
    }

    m_nodesToReparent.clear();
    m_newParent.reset();

    if (!m_nodeToAddChildTo.expired())
    {
        auto newNode = scene->AddNode("New Node");

        m_nodeToAddChildTo.lock()->AddChild(newNode);
        m_nodeToAddChildTo.reset();
    }

    if (!m_nodesToRemove.empty())
    {
        for (const auto &weakNode : m_nodesToRemove)
        {
            auto node = weakNode.lock();
            if (!node)
            {
                continue;
            }

            if (auto parent = node->GetParent().lock())
            {
                parent->RemoveChild(node);
            }

            scene->RemoveRoot(node);
        }

        m_nodesToRemove.clear();

        m_editorContext->GetInteractionState().ClearNodeSelection();
    }

    ImGui::EndDisabled();
}

void HierarchyWindow::RefreshLoadedScenes(std::vector<std::string> &p_sceneFiles)
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

void HierarchyWindow::RefreshLoadedNodes(std::vector<std::string> &p_nodeFiles)
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

void HierarchyWindow::DrawSceneWindow()
{
    static char fileName[128] = "MyScene";
    static std::weak_ptr<Droplet::Scene::Scene> previousScene;
    static bool hasPreviousScene = false;

    auto activeScene = m_editorContext->GetActiveScene().lock();
    if (activeScene && hasPreviousScene && previousScene.lock() != activeScene)
    {
        const std::string sceneName = activeScene->GetName();
        std::strncpy(fileName, sceneName.c_str(), sizeof(fileName) - 1);
        fileName[sizeof(fileName) - 1] = '\0';
    }
    previousScene = activeScene;
    hasPreviousScene = true;

    ImGui::InputText("Scene name", fileName, sizeof(fileName));

    static std::vector<std::string> sceneFiles;
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
        activeScene->SetName(fileName);
        std::string path = (scenesDirectory).string() + std::string(fileName) + ".json";
        m_editorContext->SaveScene(path);
    }

	ImGui::SameLine();

    if (ImGui::Button("New Scene"))
    {
        std::string name = "New Scene";
        m_editorContext->GetActiveScene().lock()->SetActive(false);
        m_editorContext->CreateNewScene(name);
        m_editorContext->SetActiveScene(name);

        std::strncpy(fileName, name.c_str(), sizeof(name) - 1);
        fileName[sizeof(name) - 1] = '\0';
    }

	ImGui::Separator();

    if (ImGui::BeginMenu("Loaded Scenes"))
    {
		auto &loadedScenes = m_editorContext->GetEngine().GetSceneManager().GetScenes();

		for (const auto &[sceneName, scene] : loadedScenes)
		{
			if (ImGui::MenuItem(sceneName.c_str()))
			{
                m_editorContext->GetActiveScene().lock()->SetActive(false);
				m_editorContext->SetActiveScene(sceneName);

                std::strncpy(fileName, sceneName.c_str(), sceneName.size() - 1);
                fileName[sceneName.size() - 1] = '\0';
			}
		}
		ImGui::EndMenu();
    }
}


void HierarchyWindow::DrawNode(const std::weak_ptr<Node> &p_node)
{
    if (p_node.expired())
    {
        return;
    }

    auto scrollFocus = m_editorContext->GetNodeScrollFocus();
    if (!scrollFocus.expired())
    {
        if (scrollFocus.lock() == p_node.lock())
        {
            ImGui::SetScrollHereY();
            m_editorContext->SetNodeScrollFocus({});
        }
    }

    ImGuiTreeNodeFlags flags = 
        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | 
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    if (p_node.lock()->GetChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (m_editorContext->GetInteractionState().IsNodeSelected(p_node))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool opened = ImGui::TreeNodeEx(p_node.lock().get(), flags, "%s", p_node.lock()->GetName().c_str());

	// Detect double-click to focus camera on the node
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		m_editorContext->GetInteractionState().SelectNode(p_node);
        m_editorContext->SetFocusCameraOnSelection(true);
	}

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

        if (auto pendingCollapseNode = m_pendingCollapseNode.lock(); pendingCollapseNode == p_node.lock() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
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

            m_pendingCollapseNode.reset();
        }

        if (ImGui::BeginDragDropSource())
        {
			auto node = p_node.lock();
            Node *draggedNode = node.get();

            ImGui::SetDragDropPayload("SCENE_NODE", &draggedNode, sizeof(Node *));

            const size_t draggedCount = GetDraggedNodes(m_editorContext->GetInteractionState(), node).size();

            if (draggedCount > 1)
            {
                ImGui::Text("%s (+%zu more)", node->GetName().c_str(), draggedCount - 1);
            }
            else
            {
                ImGui::Text("%s", node->GetName().c_str());
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
                auto &interactionState = m_editorContext->GetInteractionState();

                if (interactionState.IsNodeSelected(p_node))
                {
                    m_nodesToRemove = interactionState.GetSelectedRootNodes();
                }
                else
                {
                    m_nodesToRemove.push_back(p_node);
                }
            }

            if (ImGui::MenuItem("Serialize Node"))
            {
                const std::filesystem::path prefabDirectory = ROOT_PATH "/GameData/Prefabs";
                const std::string filePath = (prefabDirectory / (p_node.lock()->GetName() + ".json")).string();
                m_editorContext->SaveNode(filePath, *p_node.lock().get());
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
                Node *draggedNode = *static_cast<Node **>(payload->Data);

                if (auto draggedNodeShared = draggedNode->weak_from_this().lock())
                {
                    if (draggedNodeShared != p_node.lock() &&
                        !p_node.lock()->IsDescendantOf(draggedNodeShared))
                    {
                        m_nodesToReparent = GetDraggedNodes(
                            m_editorContext->GetInteractionState(),
                            draggedNodeShared);

                        m_newParent = p_node;
                    }
                }
            }

            if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload(PAYLOAD_COMPONENT.data()))
            {
                if (payload->DataSize == sizeof(ComponentDragDropData))
                {
                    const auto &data = *static_cast<const ComponentDragDropData *>(payload->Data);
                    auto sourceNode = data.sourceNode
                        ? data.sourceNode->weak_from_this().lock()
                        : nullptr;
                    auto destinationNode = p_node.lock();
                    if (sourceNode && destinationNode && data.component)
                    {
                        sourceNode->MoveComponent(data.component, *destinationNode);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

    if (opened)
    {
        for (const auto &child : p_node.lock()->GetChildren())
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
		ImGuiHoveredFlags hoverFlags = 
            ImGuiHoveredFlags_AllowWhenBlockedByPopup | 
            ImGuiHoveredFlags_AllowWhenBlockedByActiveItem;

        if (ImGui::IsWindowHovered(hoverFlags) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) 
            && ImGui::IsMouseHoveringRect(p_emptyAreaMin, p_emptyAreaMax)
            && !ImGui::IsAnyItemActive() && !IsOverNodeRow(io.MousePos))
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
