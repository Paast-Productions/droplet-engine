#include "SceneInspectorWindow.hpp"
#include <SceneSystem/Scene.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

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
	//
	// // Scene configurations
	std::shared_ptr<Droplet::Scene::Scene> scene = m_editorContext->GetActiveScene().lock();
	// static std::string sceneName = scene->GetName();
	// if (ImGui::InputText("Scene name", &sceneName))
	// {
	// 	scene->SetName(sceneName);
	// }
	
	ImGui::SeparatorText("Octree");
	
	// Octree configurations
	static int octreeExt = 100;
	ImGui::DragInt("Octree extent size", &octreeExt, 1, 10);
	if (ImGui::Button("Apply new octree extents"))
	{
		scene->InitializeOctree(static_cast<float>(octreeExt));
	}
	
	if (ImGui::Button("Rebuild octree"))
	{
		scene->RebuildOctree();
	}
	
	ImGui::NewLine();
	
	// Render octree options
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
	static glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	if (physics != nullptr)
	{
		gravity = JPHtoGLM(physics->GetPhysicsSystem()->GetGravity());
		if (ImGui::DragFloat3("Gravity", &gravity.x, 0.01f))
		{
			physics->GetPhysicsSystem()->SetGravity(GLMtoJPH(gravity));
		}
	}
	if (physics == nullptr && ImGui::Button("Initialize physics"))
	{
		scene->InitializePhysics();
	}
}

void Droplet::Editor::SceneInspectorWindow::RenderToolbar()
{
	
}
