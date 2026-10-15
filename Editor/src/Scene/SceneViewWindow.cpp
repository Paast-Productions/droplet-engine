#include "SceneViewWindow.hpp"
#include <SceneSystem/Scene.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <EditorConfig.hpp>

using namespace Droplet::Editor::Scene;

void SceneViewWindow::RenderImpl()
{
	ImDrawList *drawList = ImGui::GetWindowDrawList();

	// Calculate the camera view rect, so that inputs relative to the camera view rect can be collected and used during the camera update.

	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImVec2 windowPos = ImGui::GetWindowPos();

	float camAspect = m_editorCamera.GetAspect();
	float contentAspect = contentRegion.x / contentRegion.y;

	ImVec2 camSize{};
	ImVec2 camViewRectMin{};
	ImVec2 camViewRectMax{};

	if (camAspect > contentAspect) // The camera is wider than the content region, so we need to fit the camera width to the content region width.
	{
		camSize.x = contentRegion.x;
		camSize.y = contentRegion.x / camAspect;

		// Center the camera view rect vertically within the content region.
		camViewRectMin.x = cursorScreenPos.x;
		camViewRectMin.y = cursorScreenPos.y + (contentRegion.y - camSize.y) * 0.5f;
		camViewRectMax.x = camViewRectMin.x + camSize.x;
		camViewRectMax.y = camViewRectMin.y + camSize.y;
	}
	else // The camera is taller than the content region, so we need to fit the camera height to the content region height.
	{
		camSize.x = contentRegion.y * camAspect;
		camSize.y = contentRegion.y;

		// Center the camera view rect horizontally within the content region.
		camViewRectMin.x = cursorScreenPos.x + (contentRegion.x - camSize.x) * 0.5f;
		camViewRectMin.y = cursorScreenPos.y;
		camViewRectMax.x = camViewRectMin.x + camSize.x;
		camViewRectMax.y = camViewRectMin.y + camSize.y;
	}

	ImVec2 camViewPos = { 
		camViewRectMin.x - windowPos.x, 
		camViewRectMin.y - windowPos.y 
	};

	// Begin the child window that encompasses the camera view.
	ImGui::SetCursorPos(camViewPos);
	ImGui::BeginChild("CameraViewChild", camSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration);

	// Collect mouse and keyboard inputs for the camera update, if the mouse is within the camera view rect.

	bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (isWindowFocused)
	{
		[[maybe_unused]] bool isMouseHovering = ImGui::IsMouseHoveringRect(camViewRectMin, camViewRectMax);

		// Get the mouse position relative to the camera view rect.

		ImVec2 mousePos = ImGui::GetIO().MousePos; // TODO: Potential failure-point. Ensure this is in screen-space.

		ImVec2 mousePosNormalizedToCamView = { 
			(mousePos.x - camViewRectMin.x) / (camViewRectMax.x - camViewRectMin.x),
			(mousePos.y - camViewRectMin.y) / (camViewRectMax.y - camViewRectMin.y)
		};

		// TODO: Send data to camera update (mousePosNormalizedToCamView, isMouseHovering)
		m_editorCamera.Update(isMouseHovering, mousePosNormalizedToCamView);
	}

	// TODO: Wait for the renderer to finish rendering the camera view to the texture

	// Render the camera view texture
	
	// HACK:Funnily enough, rendering this nullptr renders ImGui's symbol atlas texture, which is a nice placeholder for now.
	ImTextureRef camViewTexture = nullptr; // TODO: Replace with the actual texture reference of the rendered camera view.
	
	ImGui::Image(camViewTexture, camSize, ImVec2(0, 1), ImVec2(1, 0));

	RenderImGuizmo(camViewRectMin, camViewRectMax);

	ImGui::EndChild();

	// HACK: Render a placeholder rectangle for the camera view until the actual camera view texture is available.
	drawList->AddRect(camViewRectMin, camViewRectMax, IM_COL32(255, 0, 0, 128), 0.0f, 0, 4.0f);
}

void SceneViewWindow::RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax)
{
	ZoneScoped;

	using DScene = Droplet::Scene::Scene;
	using DNode = Droplet::Scene::Node;
	using DTransform = Droplet::Scene::Transform;

	// Setup
	ImVec2 rectPos = p_sceneViewMin;
	ImVec2 rectSize = p_sceneViewMax - p_sceneViewMin;

	ImGuizmo::SetRect(rectPos.x, rectPos.y, rectSize.x, rectSize.y);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	glm::mat4 tempViewMat = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 tempProjMat = glm::perspective(m_editorCamera.GetFOV(), m_editorCamera.GetAspect(), 0.1f, 100.0f);

	std::shared_ptr<DScene> selectedScene = GetEngine()->GetSceneManager().GetScene("TestScene");
	std::vector<std::shared_ptr<DNode>> nodes = selectedScene->GetRoots();

	// Render ImGuizmo cubes for all nodes in the scene
	{
		std::vector<glm::mat4> tempMatrices{};

		std::function<void(const std::shared_ptr<DNode> &)> recurseNodes = [&](const std::shared_ptr<DNode> &node) {
			if (!node)
			{
				return;
			}

			tempMatrices.push_back(node->GetTransform().GetMatrix(DTransform::Space::World));

			for (const auto &child : node->GetChildren())
			{
				recurseNodes(child);
			}
			};

		for (const auto &node : nodes)
		{
			recurseNodes(node);
		}

		if (tempMatrices.size() > 0)
		{
			ImGuizmo::DrawCubes(&(tempViewMat[0][0]), &(tempProjMat[0][0]), (float*)&(tempMatrices[0][0]), tempMatrices.size());
		}
	}

	// Render transform gizmo for selected node
	{
		std::vector<std::shared_ptr<DNode>> selectedNodes = m_interactionState->GetSelectedNodes();

		// TODO: Make this configurable by the user
		ImGuizmo::OPERATION currentOperation =
			ImGuizmo::TRANSLATE |
			ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z |
			ImGuizmo::BOUNDS;

		if (selectedNodes.size() > 0)
		{
			std::shared_ptr<DNode> selectedNode = selectedNodes[0];

			Math::AABB nodeAABB = selectedNode->GetBounds()->GetAABB();
			glm::vec3 aabb[2] = { nodeAABB.GetMin(), nodeAABB.GetMax() };

			glm::mat4 nodeMatrix = selectedNode->GetTransform().GetMatrix(DTransform::Space::World);
			glm::mat4 deltaMatrix = glm::mat4(1.0f);

			ImGuizmo::MODE manipulateSpace = (ImGuizmo::MODE)EditorConfig::LoadInt("manipulateSpace", 0);

			bool result = ImGuizmo::Manipulate(
				&(tempViewMat[0][0]),	// View matrix
				&(tempProjMat[0][0]),	// Projection matrix
				currentOperation,		// Operation
				manipulateSpace,		// Space
				&(nodeMatrix[0][0]),	// Matrix to manipulate
				&(deltaMatrix[0][0]),	// Output delta matrix
				nullptr,				// Optional snap values
				&(aabb[0].x),			// Optional bounds
				nullptr					// Optional bounds snap values
			);

			if (result)
			{
				//selectedNode->GetTransform().TransformByMatrix(deltaMatrix, DTransform::Space::World);

				selectedNode->GetTransform().SetMatrix(nodeMatrix, DTransform::Space::World);
			}
		}
	}
}
