#include "SceneInspectorWindow.hpp"
#include <SceneSystem/Scene.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <NodeSlot.hpp>

using namespace Droplet::Scene;

void Droplet::Editor::SceneInspectorWindow::InitImpl()
{
	SetName("Scene Inspector");
}

void Droplet::Editor::SceneInspectorWindow::CloseImpl()
{
	
}

void Droplet::Editor::SceneInspectorWindow::RenderImpl()
{
	// ImGui::SeparatorText("Scene");

	// Scene configurations
	std::shared_ptr<Droplet::Scene::Scene> scene = m_editorContext->GetActiveScene().lock();
	if (scene == nullptr || !scene->IsLoaded())
	{
		return;
	}
	
	// static std::string sceneName = scene->GetName(); // Changing scene name doesn't work
	// if (ImGui::InputText("Scene name", &sceneName))
	// {
	// 	scene->SetName(sceneName);
	// }
	
	ImGui::SeparatorText("Octree");
	Octree &octree = scene->GetOctree();
	
	static int octreeExt = static_cast<int>(octree.GetExtentLength());
	static int maxNodes = octree.GetMaxNodes();
	static int maxDepth = octree.GetMaxDepth();
	
	// Update config display when changing scene
	static std::string_view tempStr = scene->GetName();
	if (tempStr != scene->GetName())
	{
		tempStr = scene->GetName();
		octreeExt = static_cast<int>(octree.GetExtentLength());
		maxNodes = octree.GetMaxNodes();
		maxDepth = octree.GetMaxDepth();
	}
	
	// Octree configurations
	ImGui::DragInt("Octree extent size", &octreeExt, 1, 10);
	if (ImGui::Button("Apply new octree extents"))
	{
		scene->InitializeOctree(static_cast<float>(octreeExt));
	}
	
	ImGui::DragInt("Octree max nodes", &maxNodes, 1, 8);
	if (ImGui::Button("Apply new max nodes"))
	{
		octree.SetMaxNodes(static_cast<std::uint8_t>(maxNodes));
		scene->RebuildOctree();
	}
	
	ImGui::DragInt("Octree max depth", &maxDepth, 1, 1);
	if (ImGui::Button("Apply new max depth"))
	{
		octree.SetMaxDepth(static_cast<std::uint8_t>(maxDepth));
		scene->RebuildOctree();
	}
	
	if (ImGui::Button("Rebuild octree"))
	{
		scene->RebuildOctree();
	}
	
	ImGui::NewLine();
	
	// Render octree options
	ImGui::Text("Render options");
	static bool rOctant = true;
	static bool rNode = true;
	ImGui::Checkbox("Octant", &rOctant);
	ImGui::SameLine();
	ImGui::Checkbox("Node", &rNode);
	
	static bool renderOctree = false;
	ImGui::Checkbox("Render Octree", &renderOctree);
	if (renderOctree)
	{
		scene->RenderOctree(rOctant, rNode);
	}
	
	ImGui::SeparatorText("Physics");
	
	// Physics configurations
	std::shared_ptr<Physics> physics = scene->GetPhysics().lock();
	if (physics != nullptr)
	{
		static glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);
		gravity = JPHtoGLM(physics->GetPhysicsSystem()->GetGravity());
		if (ImGui::DragFloat3("Gravity", &gravity.x, 0.01f))
		{
			physics->GetPhysicsSystem()->SetGravity(GLMtoJPH(gravity));
		}
	}
	else if (ImGui::Button("Initialize physics"))
	{
		scene->InitializePhysics();
	}
}

void Droplet::Editor::SceneInspectorWindow::RenderToolbar()
{
	
}
