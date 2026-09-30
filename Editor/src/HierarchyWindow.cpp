#include "HierarchyWindow.hpp"
#include "InteractionState.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>


using namespace Droplet::Editor;
using namespace Droplet::Scene;

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

    if (ImGui::IsItemClicked())
    {
        m_interactionState->ClearNodeSelection();
        m_interactionState->SelectNode(p_node);
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