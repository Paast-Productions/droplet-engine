#include "HierarchyWindow.hpp"

#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>



using namespace Droplet::Editor;
using namespace Droplet::Scene;

void HierarchyWindow::InitImpl()
{
	// Create a test scene manager and load a test scene

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
    }
    
    const auto scene = m_testSceneManager->GetScene("TestScene");
    //const auto sceneManager = m_instance->GetSceneManager();

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

    const bool opened = ImGui::TreeNodeEx(p_node.get(), flags, "%s", p_node->GetName().c_str());

    if (opened)
    {
        for (const auto &child : p_node->GetChildren())
        {
            DrawNode(child);
        }

        ImGui::TreePop();
    }
}