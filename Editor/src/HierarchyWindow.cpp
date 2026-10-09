#include "HierarchyWindow.hpp"
#include "InteractionState.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Component.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
//#include <Engine/Engine.hpp>
#include <memory>


using namespace Droplet::Editor;
using namespace Droplet::Scene;

Droplet::Editor::HierarchyWindow::HierarchyWindow(std::shared_ptr<Droplet::Engine> p_instance, std::shared_ptr<InteractionState> p_interactionState)
{
	m_instance = p_instance;
	m_interactionState = p_interactionState;
}

void HierarchyWindow::InitImpl()
{   
    SetName("Node Hierarchy");

	m_instance->GetSceneManager().LoadScene("TestScene");

	m_selectedScene = m_instance->GetSceneManager().GetScene("TestScene"); 
}

void HierarchyWindow::RenderImpl()
{
    if (!m_instance)
    {
        return;
    }
    
    if (!m_selectedScene)
    {
        return;
    }

    if (ImGui::Button("Add Node"))
    {
        m_selectedScene->AddNode("New Node");
    }

    ImGui::Separator();

    for (const auto &root : m_selectedScene->GetRoots())
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
                Node *draggedNode =
                    *static_cast<Node **>(payload->Data);

                auto oldParent = draggedNode->shared_from_this()->GetParent();
                if (oldParent)
                {
                    oldParent->RemoveChild(draggedNode->shared_from_this());
                }
            }

            ImGui::EndDragDropTarget();
        }
    }

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
        auto newNode = m_selectedScene->AddNode("New Node");

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
		m_selectedScene->RemoveRoot(m_nodeToRemove);
		m_nodeToRemove.reset();
    }
} 

void HierarchyWindow::DrawNode(const std::shared_ptr<Node> &p_node)
{
    if (!p_node)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (p_node->GetChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (m_interactionState->IsNodeSelected(p_node))
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
            m_interactionState->ClearNodeSelection();
            m_interactionState->SelectNode(p_node);
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