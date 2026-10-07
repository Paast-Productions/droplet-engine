#include "HierarchyWindow.hpp"
#include "InteractionState.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
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
	// Create a test scene manager and load a test scene
	//TODO: Remove this when the hierarchy window is integrated engine instance
    m_testSceneManager = std::make_shared<SceneManager>();

    m_testSceneManager->LoadScene("TestScene");
    
    const auto scene = m_testSceneManager->GetScene("TestScene");

    auto root = scene->AddNode("Root");    

    auto player = root->AddChild(scene->AddNode("Player"));

    auto camera = player->AddChild(scene->AddNode("Camera"));

    auto weapon = player->AddChild(scene->AddNode("Weapon"));

    auto enemy = root->AddChild(scene->AddNode("Enemy"));
}

void HierarchyWindow::RenderImpl()
{
    if (!m_instance)
    {
        //return;
		//TODO: This should return when the hierarchy window is integrated with the engine instance
    }
    
    const auto scene = m_testSceneManager->GetScene("TestScene");
    //const auto sceneManager = m_instance->GetSceneManager();
	//TODO: Get the currently viewed scene from the engine instance instead of using the test scene manager

    if (!scene)
    {
        return;
    }

    for (const auto &root : scene->GetRoots())
    {
        DrawNode(root);
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

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCENE_NODE"))
            {
                Node *draggedNode = *static_cast<Node **>(payload->Data);

                auto draggedNodeShared = draggedNode->shared_from_this();

                auto oldParent = draggedNodeShared->GetParent();

                if (oldParent)
                {
                    oldParent->RemoveChild(draggedNodeShared);
                }

                p_node->AddChild(draggedNodeShared);
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