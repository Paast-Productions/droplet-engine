#include <SceneViewWindow.hpp>
#include <math/bounds/Intersection.hpp>
#include <math/bounds/Conversion.hpp>
#include <SceneSystem/Scene.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <tracy/public/tracy/Tracy.hpp>
#include <EditorConfig.hpp>
#include <EditorShortcuts.hpp>
#include <Engine/EngineFlags.hpp>
#include <IconFontCppHeaders/IconsLucide.h>
#include <chrono>
#include <iostream>
#include <Graphics/DebugDraw.hpp>

using namespace Droplet::Editor::Scene;
using namespace Droplet::Graphics::Debug;

void SceneViewWindow::InitImpl()
{
	
	SetName("Scene View"); 
	//ToggleToolbar(true);
	
	// TODO: MOVE THIS INTO THE RENDERER
	m_renderTarget = m_editorContext->GetEngine().CreateRenderTarget(720, 720);
	m_textureDescriptorSet = m_editorContext->GetEngine().RegisterImGuiTexture(m_renderTarget);
}

void SceneViewWindow::RenderToolbar()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::BeginMenu("Layout"))
		{
			if (ImGui::MenuItem("Load Default Layout"))
			{
				ImGui::LoadIniSettingsFromDisk(ROOT_PATH "/Editor/Default.ini");

			}

			if (ImGui::MenuItem("Save Default Layout"))
			{
				ImGui::SaveIniSettingsToDisk(ROOT_PATH "/Editor/Default.ini");
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void SceneViewWindow::RenderImpl()
{
	ImDrawList *drawList = ImGui::GetWindowDrawList();

	// Temporarily time variable to create deltaTime for our update
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

	static TimePoint s_lastFrameTime{ std::chrono::high_resolution_clock::now() };
	const TimePoint  currentTime{ std::chrono::high_resolution_clock::now() };
	const float deltaTime = { std::chrono::duration<float>(currentTime - s_lastFrameTime).count() };
	s_lastFrameTime = currentTime;

	// Calculate the camera view rect, so that inputs relative to the camera view rect can be collected and used during the camera update.

	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImVec2 windowPos = ImGui::GetWindowPos();

	float camAspect = m_editorContext->GetEditorCamera().GetAspect();
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

	// Process shortcut key presses
	if (ImGui::IsWindowFocused())
	{
		EditorShortcuts::ProcessSceneViewKeyShortcuts();
	}

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

		if (isWindowFocused && isMouseHovering)
		{
			ImVec2 center = {
				(camViewRectMin.x + camViewRectMax.x) * 0.5f,
				(camViewRectMin.y + camViewRectMax.y) * 0.5f
			};

			ImGuiViewport *viewport = ImGui::GetWindowViewport();

			SDL_Window *sceneWindow = nullptr;

			if (viewport)
			{
				SDL_WindowID windowID = static_cast<SDL_WindowID>(reinterpret_cast<uintptr_t>(viewport->PlatformHandle));
				sceneWindow = SDL_GetWindowFromID(windowID);
			}

			const EditorCamera &camera = m_editorContext->GetEditorCamera();
			EditorCameraController &controller = m_editorContext->GetEditorCameraController();
			GameInput &input = GameInput::Get();

			if (m_cameraLookToggled)
			{
				controller.Update(deltaTime, sceneWindow, center.x, center.y);
			}

			// Mouse selection of nodes in the scene view
			if (input.MousePressed(Mouse::LMB) && !m_usingGizmo && !m_hoveringGizmo)
			{
				glm::vec2 mousePosInView = glm::vec2(mousePosNormalizedToCamView.x, mousePosNormalizedToCamView.y);
				Math::Ray viewRay = camera.GetViewRay(mousePosInView);

				// Perform raycast against the scene
				std::weak_ptr<Droplet::Scene::Scene> scene = m_editorContext->GetActiveScene();

				if (!scene.expired())
				{
					Math::RayHit hit{};
					auto hitNode = scene.lock()->RaycastNode(viewRay, &hit);

					InteractionState &interactionState = m_editorContext->GetInteractionState();

					if (hit.didHit && !hitNode.expired())
					{
						if (interactionState.IsNodeSelected(hitNode.lock()))
						{
							// Deselect the hit node if it is already selected
							interactionState.DeselectNode(hitNode.lock());
						}
						else
						{
							// Select the hit node
							interactionState.SelectNode(hitNode.lock());
						}
					}
					else
					{
						// Deselect all nodes if no hit
						interactionState.ClearNodeSelection();
					}
				}
			}

			if (input.KeyPressed(Key::KeyQ))
			{
				m_cameraLookToggled = !m_cameraLookToggled;

				controller.CameraLock(m_cameraLookToggled);
			}
		}
	}

	// TODO: Wait for the renderer to finish rendering the camera view to the texture
	ImTextureRef camViewTexture = m_textureDescriptorSet;

	// Render the camera view texture to the scene view window
	ImGui::Image(camViewTexture, camSize, ImVec2(0, 0), ImVec2(1, 1));
	
	RenderImGuizmo(camViewRectMin, camViewRectMax);

	// Render play/pause button
	{
		ImVec2 buttonSize = ImVec2(30, 30);
		ImVec2 buttonPos = ImVec2(
			camViewRectMin.x + camSize.x * 0.5f - buttonSize.x - 10,
			camViewRectMin.y + 10
		);

		bool isPlaying = EngineFlagsOwner::GetUpdateFlags() == Droplet::UpdateFlags::PlayModeFlags;

		ImGui::SetCursorScreenPos(buttonPos);
		if (isPlaying)
		{
			// Make red
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

			if (ImGui::Button(ICON_LC_PAUSE "##PauseButton", buttonSize))
			{
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::EditorFlags);
			}

			ImGui::PopStyleColor(4);
		}
		else
		{
			// Make green
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			if (ImGui::Button(ICON_LC_PLAY "##PlayButton", buttonSize))
			{
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::PlayModeFlags);
			}

			ImGui::PopStyleColor(4);
		}
	}
	
	ImGui::EndChild();
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

	// Pre-fetch data
	InteractionState &interactionState = m_editorContext->GetInteractionState();

	const EditorCamera &camera = m_editorContext->GetEditorCamera();
	glm::mat4 camViewMat = camera.GetViewMatrix();
	glm::mat4 camProjMat = camera.GetProjectionMatrix();

	std::shared_ptr<DScene> selectedScene = m_editorContext->GetEngine().GetSceneManager().GetScene("TestScene"); // TODO: Make weak pointer
	std::vector<std::shared_ptr<DNode>> nodes = selectedScene->GetRoots(); // TODO: Make weak pointer

	// DebugDraw wireframe around selected nodes by their bounds
	{
		auto selectedNodes = interactionState.GetSelectedNodes(); // TODO: Make weak pointer

		for (const auto &node : selectedNodes)
		{
			if (!node)
			{
				continue;
			}

			glm::mat4 nodeMat = node->GetTransform().GetMatrix(DTransform::Space::World);
			Math::OBB nodeOBB = node->GetBounds()->GetTransformedOBB(nodeMat);

			DebugDraw::DrawBox(
				nodeOBB,
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				true, true
			);
		}
	}

	bool didUse = false;

	// Render transform gizmo for selected node
	{
		std::vector<std::shared_ptr<DNode>> selectedNodes = m_editorContext->GetInteractionState().GetSelectedNodes(); // TODO: Make weak pointer

		// TODO: Make this configurable by the user
		ImGuizmo::OPERATION currentOperation = static_cast<ImGuizmo::OPERATION>(EditorConfig::LoadInt("manipulateOperation", 0));

		if (!selectedNodes.empty())
		{
			const std::shared_ptr<DNode> &selectedNode = selectedNodes[0]; // TODO: Make weak pointer

			Math::AABB nodeAABB = selectedNode->GetBounds()->GetAABB();
			glm::vec3 aabb[2] = { nodeAABB.GetMin(), nodeAABB.GetMax() };

			// If bounds should be active
			float *bounds = nullptr;
			if (EditorConfig::LoadBool("manipulateBounds", false))
			{
				bounds = &(aabb[0].x);
			}

			glm::mat4 nodeMatrix = selectedNode->GetTransform().GetMatrix(DTransform::Space::World);
			glm::mat4 deltaMatrix = glm::mat4(1.0f);

			ImGuizmo::MODE manipulateSpace = (ImGuizmo::MODE)EditorConfig::LoadInt("manipulateSpace", 0);

			bool result = ImGuizmo::Manipulate(
				&(camViewMat[0][0]),	// View matrix
				&(camProjMat[0][0]),	// Projection matrix
				currentOperation,		// Operation
				manipulateSpace,		// Space
				&(nodeMatrix[0][0]),	// Matrix to manipulate
				&(deltaMatrix[0][0]),	// Output delta matrix
				nullptr,				// Optional snap values
				bounds,					// Optional bounds
				nullptr					// Optional bounds snap values
			);

			if (result)
			{
				selectedNode->GetTransform().SetMatrix(nodeMatrix, DTransform::Space::World);
				didUse = true;
			}
		}
	}

	m_usingGizmo = ImGuizmo::IsUsingAny() || didUse;
	m_hoveringGizmo = ImGuizmo::IsOver() || ImGuizmo::IsOverBounds();
}

Droplet::Graphics::RenderTarget &SceneViewWindow::GetRenderTarget()
{			
	return m_renderTarget;
}

void SceneViewWindow::SetViewDescriptor(VkDescriptorSet p_textureDescriptor)
{
	m_textureDescriptorSet = p_textureDescriptor;
}