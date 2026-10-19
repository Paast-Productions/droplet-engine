#include "SceneViewWindow.hpp"
#include <math/bounds/Intersection.hpp>
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
	
	// TODO: Make rendertarget reziable and match ImGui window
	m_renderTarget = m_editorContext->GetRenderer().CreateRenderTarget(720, 720);
	m_textureDescriptor = m_editorContext->GetRenderer().RegisterImGuiTexture(*m_renderTarget);
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

		// TODO: Send data to camera update (mousePosNormalizedToCamView, isMouseHovering)
		//m_editorCamera.Update(isMouseHovering, mousePosNormalizedToCamView);

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
				RenderImGuizmo(camViewRectMin, camViewRectMax);

				sceneWindow = SDL_GetWindowFromID(windowID);

				// HACK: Render a placeholder rectangle for the camera view until the actual camera view texture is available.
				//drawList->AddRect(camViewRectMin, camViewRectMax, IM_COL32(255, 0, 0, 128), 0.0f, 0, 4.0f);
			}

			GameInput &input = GameInput::Get();
			EditorCameraController &controller = m_editorContext->GetEditorCameraController();
			if (m_cameraLookToggled)
			{
				controller.Update(deltaTime, sceneWindow, center.x, center.y);
				
				// create ray that is used for raycasting
				if (input.MousePressed(Mouse::LMB)) 
				{
					const EditorCamera &camera = m_editorContext->GetEditorCamera();
					glm::vec3 position = camera.GetPosition();
					glm::vec3 front = camera.GetForward();

					float x = mousePosNormalizedToCamView.x * 2.0f - 1.0f;
					float y = 1.0f - mousePosNormalizedToCamView.y * 2.0f;
					glm::vec4 nearClip(x, y, 0.0f, 1.0f);
					glm::vec4 farClip(x, y, 1.0f, 1.0f);

					glm::mat4 inverseProjection = glm::inverse(camera.GetProjectionMatrix());

					glm::vec4 nearView = inverseProjection * nearClip;
					glm::vec4 farView = inverseProjection * farClip;

					nearView /= nearView.w;
					farView /= farView.w;

					glm::mat4 inverseView = glm::inverse(camera.GetViewMatrix());
					glm::vec4 nearWorld = inverseView * nearView;
					glm::vec4 farWorld = inverseView * farView;

					// the ray in question
					glm::vec3 direction = glm::normalize(
						glm::vec3(farWorld) -
						glm::vec3(nearWorld)
					);
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
			if (ImGui::Button(ICON_LC_PAUSE "##PauseButton", buttonSize))
			{
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::EditorFlags);
			}
		}
		else
		{
			if (ImGui::Button(ICON_LC_PLAY "##PlayButton", buttonSize))
			{
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::PlayModeFlags);
			}
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

	// Camera
	const EditorCamera &camera = m_editorContext->GetEditorCamera();
	glm::mat4 camViewMat = camera.GetViewMatrix();
	glm::mat4 camProjMat = camera.GetProjectionMatrix();

	std::shared_ptr<DScene> selectedScene = m_editorContext->GetEngine().GetSceneManager().GetScene("TestScene");
	std::vector<std::shared_ptr<DNode>> nodes = selectedScene->GetRoots();

	// Render ImGuizmo cubes for all nodes in the scene
	{
		Math::Frustum camFrustum = camera.GetFrustum();

		std::function<void(const std::shared_ptr<DNode> &)> recurseNodes = [&](const std::shared_ptr<DNode> &node) {
			if (!node)
			{
				return;
			}

			if (!node->IsActive())
			{
				return;
			}

			glm::mat4 nodeMat = node->GetTransform().GetMatrix(DTransform::Space::World);

			Math::OBB baseOBB = Math::OBB(glm::vec3(0.0f), glm::vec3(0.5f), glm::mat3(1.0f));
			Math::OBB nodeOBB = Math::OBB::Transform(baseOBB, nodeMat);

			Math::IntersectType intersection = Math::Intersects(camFrustum, nodeOBB);

			if (intersection != Math::IntersectType::None)
			{
				// Draw
				glm::vec4 nodeColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

				// Randomize color based on the nodes pointer to ensure it's consistent across frames
				std::hash<std::shared_ptr<DNode>> hasher;
				size_t hashValue = hasher(node);

				srand(static_cast<unsigned int>(hashValue));
				int r = rand() % 256;
				int g = rand() % 256;
				int b = rand() % 256;

				DebugDraw::DrawBox(
					nodeOBB, 
					glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f),
					true//, true
				);
			}

			for (const auto &child : node->GetChildren())
			{
				recurseNodes(child);
			}
		};

		for (const auto &node : nodes)
		{
			recurseNodes(node);
		}
	}

	// Render transform gizmo for selected node
	{
		std::vector<std::shared_ptr<DNode>> selectedNodes = m_editorContext->GetInteractionState().GetSelectedNodes();

		// TODO: Make this configurable by the user
		ImGuizmo::OPERATION currentOperation = (ImGuizmo::OPERATION)EditorConfig::LoadInt("manipulateOperation", 0);

		if (selectedNodes.size() > 0)
		{
			std::shared_ptr<DNode> selectedNode = selectedNodes[0];

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
			}
		}
	}
}
Droplet::Graphics::RenderTarget &SceneViewWindow::GetRenderTarget()
{			
	return *m_renderTarget;
}