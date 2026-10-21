#include "SceneViewWindow.hpp"
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
	ToggleToolbar(true);
	
	// TODO: 
	//     Move this to the renderer and camera class. 
	//     The camera should request the renderer to create a render target.
	m_renderTarget = m_editorContext->GetEngine().CreateRenderTarget(720, 720);
	m_textureDescriptorSet = m_editorContext->GetEngine().RegisterImGuiTexture(m_renderTarget);
}

void Droplet::Editor::Scene::SceneViewWindow::RenderToolbar()
{
	if (ImGui::BeginMenu("Space"))
	{
		ImGuizmo::MODE currentMode = (ImGuizmo::MODE)EditorConfig::LoadInt("manipulateSpace", 0);
		ImGuizmo::MODE worldMode = ImGuizmo::WORLD;
		ImGuizmo::MODE localMode = ImGuizmo::LOCAL;

		if (ImGui::MenuItem("World", nullptr, currentMode == worldMode))
		{
			EditorConfig::StoreInt("manipulateSpace", worldMode);
		}

		if (ImGui::MenuItem("Local", nullptr, currentMode == localMode))
		{
			EditorConfig::StoreInt("manipulateSpace", localMode);
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Operation"))
	{
		ImGuizmo::OPERATION currentOperation = (ImGuizmo::OPERATION)EditorConfig::LoadInt("manipulateOperation");
		ImGuizmo::OPERATION translationOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::OPERATION rotationOperation = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z;
		ImGuizmo::OPERATION scaleOperation = ImGuizmo::SCALE;
		ImGuizmo::OPERATION boundsOperation = ImGuizmo::BOUNDS;
		ImGuizmo::OPERATION universalOperation = translationOperation | rotationOperation;

		if (ImGui::MenuItem("Universal", nullptr, currentOperation == universalOperation))
		{
			EditorConfig::StoreBool("manipulateBounds", true);
			EditorConfig::StoreInt("manipulateOperation", universalOperation);
		}

		if (ImGui::MenuItem("Translation", nullptr, currentOperation == translationOperation))
		{
			EditorConfig::StoreBool("manipulateBounds", false);
			EditorConfig::StoreInt("manipulateOperation", translationOperation);
		}

		if (ImGui::MenuItem("Rotation", nullptr, currentOperation == rotationOperation))
		{
			EditorConfig::StoreBool("manipulateBounds", false);
			EditorConfig::StoreInt("manipulateOperation", rotationOperation);
		}

		if (ImGui::MenuItem("Scale", nullptr, currentOperation == scaleOperation))
		{
			EditorConfig::StoreBool("manipulateBounds", false);
			EditorConfig::StoreInt("manipulateOperation", scaleOperation);
		}

		if (ImGui::MenuItem("Bounds", nullptr, currentOperation == boundsOperation))
		{
			EditorConfig::StoreBool("manipulateBounds", true);
			EditorConfig::StoreInt("manipulateOperation", boundsOperation);
		}

		ImGui::EndMenu();
	}

	float gizmoSize = EditorConfig::LoadFloat("GizmoSize", 0.1f);
	if (ImGui::BeginMenu("GizmoSize"))
	{
		ImGui::SliderFloat("Gizmo size", &gizmoSize, 0.001, 1);
		EditorConfig::StoreFloat("GizmoSize", gizmoSize);
		ImGuizmo::SetGizmoSizeClipSpace(gizmoSize);
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
	
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImVec2 windowPos = ImGui::GetWindowPos();
	
	GameInput &input = GameInput::Get();
	float contentAspect = contentRegion.x / contentRegion.y;
	vk::Extent2D extent = m_renderTarget->GetExtent();

	uint32_t width = static_cast<uint32_t>(contentRegion.x); 
	uint32_t height = static_cast<uint32_t>(contentRegion.y);
	// If mouse is released and viewSize is changed, Update the camera and renderTarget
	if (input.MouseReleased(Mouse::LMB))
	{	
		if (width != extent.width || height != extent.height)
		{
			if (contentRegion.x <= 0.0f || contentRegion.y <= 0.0f)
			{
				return;
			}
			if (input.MouseReleased(Mouse::LMB))
			{
				m_editorContext->GetEditorCamera().SetAspect(contentAspect);
				UpdateRenderTarget(width, height);
			}
		}
	}
	
	ImVec2 camSize = contentRegion;
	
	ImVec2 camViewRectMin = {
		cursorScreenPos.x + (contentRegion.x - camSize.x) * 0.5f,
		cursorScreenPos.y + (contentRegion.y - camSize.y) * 0.5f,
	};
	
	ImVec2 camViewRectMax = {
		camViewRectMin.x  + camSize.x,
		camViewRectMin.y + camSize.y
	};

	ImVec2 camViewPos = { 
		camViewRectMin.x - windowPos.x, 
		camViewRectMin.y - windowPos.y 
	};
	
	// Begin the child window that encompasses the camera view.
	ImGui::SetCursorPos(camViewPos);
	ImGui::BeginChild("CameraViewChild", camSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration);

	// Process shortcut key presses
	if (ImGui::IsWindowFocused() && !m_cameraLookToggled)
	{
		EditorShortcuts::ProcessSceneViewKeyShortcuts();
	}

	ImTextureRef camViewTexture = m_textureDescriptor;

	// Render the camera view texture to the scene view window
	ImGui::Image(camViewTexture, camSize, ImVec2(0, 0), ImVec2(1, 1));

	// Check if the mouse is hovering over the scene view
	m_hoveringSceneView = ImGui::IsItemHovered();

	bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (isWindowFocused)
	{
		bool isMouseHovering = ImGui::IsMouseHoveringRect(camViewRectMin, camViewRectMax);

		GameInput &input = GameInput::Get();
		const EditorCamera &camera = m_editorContext->GetEditorCamera();
		EditorCameraController &controller = m_editorContext->GetEditorCameraController();

		// Get the mouse position relative to the camera view rect
		ImVec2 mousePos = ImGui::GetIO().MousePos;

		ImVec2 mousePosNormalizedToCamView = {
			(mousePos.x - camViewRectMin.x) / (camViewRectMax.x - camViewRectMin.x),
			(mousePos.y - camViewRectMin.y) / (camViewRectMax.y - camViewRectMin.y)
		};

		if (isMouseHovering)
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

			if (m_cameraLookToggled)
			{
				controller.Update(deltaTime, sceneWindow, center.x, center.y);
			}
		}

		if (m_hoveringSceneView)
		{
			// Mouse selection of nodes in the scene view
			if (input.MousePressed(Mouse::LMB) && !m_usingGizmo && !m_hoveringGizmo && !m_hoveringPlayButton)
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

	ImTextureRef camViewTexture = m_textureDescriptor;

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

		m_hoveringPlayButton = ImGui::IsItemHovered();
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

	std::shared_ptr<DScene> selectedScene = m_editorContext->GetActiveScene(); // TODO: Make weak pointer
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

void SceneViewWindow::UpdateRenderTarget(uint32_t width, uint32_t height)
{
	auto &renderer = m_editorContext->GetRenderer();
	
	renderer.WaitIdle();
	
	if (m_textureDescriptor != VK_NULL_HANDLE)
	{
		ImGui_ImplVulkan_RemoveTexture(m_textureDescriptor);
		m_textureDescriptor = VK_NULL_HANDLE;
	}
	
	m_renderTarget = renderer.CreateRenderTarget(width, height);
	
	m_textureDescriptor = renderer.RegisterImGuiTexture(*m_renderTarget);
	
}